
// Тема 5. Учебный пример: игра "Бильярд"
//
// Программа имитирует движение шаров по бильярдному со стенками и лузами.
// 
#include <iostream>
#include <cmath>
#include <vector>

#include <windows.h>		// Заголовочный файл с описаниями функций Windows

#include "GL/gl.h"			// Заголовочные файлы библиотеки OpenGL и
#include "GL/glu.h"			// вспомогательных библиотек (эти файлы
#include "GL/glaux.h"		// лежат в каталоге проекта в подкаталоге GL)


// ********************************************************************************
//   Константы и глобальные переменные
// ********************************************************************************

const double PI = 3.14159265359;
const int    WINDOW_WIDTH = 400;	// Ширина экранного окна OpenGL
const int    WINDOW_HEIGHT = 300;	// и высота (в пикселах)

class RedGreenBlue
{
private:

	unsigned char red = 0;
	unsigned char green = 0;
	unsigned char blue = 0;

public:

	RedGreenBlue(unsigned char red_, unsigned char green_, unsigned char blue_)
		: red(red_), green(green_), blue(blue_) {}

	unsigned char GetRed() const
	{
		return red;
	}

	unsigned char GetGreen() const
	{
		return green;
	}

	unsigned char GetBlue() const
	{
		return blue;
	}
};

// Опережающие объявления типов данных, чтобы можно было 
// создать переменные-указатели на эти типы
class GraphicalObject;
class Hole;
class Wall;
class Ball;

// Глобальные переменные, совместно представляющие модель бильярда
bool   fBallMoved = false;		// Флаг наличия хотя бы одного движущегося шара
int	   saveRack = 0;			// Счетчик выбитых шаров

Ball* pCueBall = nullptr;	// Указатель на белый шар (в списке от тоже есть)


std::vector<GraphicalObject*> listOfObject{};

// ********************************************************************************
//   Определения классов
// ********************************************************************************

// Вспомогательный класс "Прямоугольник"
class Rect {
public:
	// Общедоступные атрибуты
	int x1, y1, x2, y2;

public:
	// Конструкторы
	Rect() : x1(0), y1(0), x2(0), y2(0) {}
	Rect(int nx1, int ny1, int nx2, int ny2) :
		x1(nx1), y1(ny1), x2(nx2), y2(ny2) {}

	// Инициализация координат прямоугольника
	void SetRect(int left, int top, int right, int bottom)
	{
		x1 = left; y1 = top; x2 = right; y2 = bottom;
	}
	// Смещение прямоугольника
	void OffsetRect(int dx, int dy)
	{
		x1 += dx; y1 += dy; x2 += dx; y2 += dy;
	}
	// Получение координат центра прямоугольника
	void CenterPoint(int& x, int& y) const
	{
		x = (x1 + x2) / 2; y = (y1 + y2) / 2;
	}
	// Проверка на совпадение координат углов прямоугольника
	bool IsRectEmpty() const { return x1 == x2 && y1 == y2; }
	// Получение ширины прямоугольника
	int Width() const { return x2 - x1 + 1; }
	// Получение высоты прямоугольника
	int Height() const { return y2 - y1 + 1; }
	// Функция вычисляет пересечение объекта с прямоугольником another
	Rect IntersectRect(const Rect& another) const;
	// Нормализация координат прямоугольника
	void NormalizeRect();
};

class GraphicalObject {
public:
	GraphicalObject(int left, int top, int right, int bottom)
	{
		region = Rect(left, top, right, bottom);
	}
	// Операции, выполняемые графическими объектами 
	virtual void Draw() const {}
	virtual void Update() {}
	virtual void HitBy(Ball& ball) {}
	// Функции-члены для доступа к переменным класса 
	Rect GetRegion() const { return region; }
protected:
	Rect region;                          // Экранная область
};


// Класс "Бильярдный шар"
class Ball : public GraphicalObject
{
public:
	Ball(int x, int y, bool fc, RedGreenBlue rgb);
	// Запрет копирующего конструктора и операции присваивания 
	// с помощью ключевого слова delete 
	Ball(const Ball&) = delete;
	Ball& operator=(const Ball&) = delete;


	// Рисование шара в текущем положении
	void Draw() const override;



	// Изменение положения шара в предположении, что с момента 
	// предыдущего изменения прошел единичный промежуток времени
	void Update() override;
	// Извещение шара о том, что в него попал другой шар
	void HitBy(Ball& ball) override;
	// Расчет угла между осью OX и направлением от центра шара до точки,
	// смещенной от центра шара на (dx, dy)
	double HitAngle(double dx, double dy) const;

	// Функции-члены для доступа к переменным класса
	void   SetEnergy(double v) { energy = v; }
	void   SetCenter(int newx, int newy);
	void   SetDirection(double newDir) { direction = newDir; };

	double GetEnergy() const { return energy; }
	void   GetCenter(int& x, int& y) const;
	double GetDirection() const { return direction; }
	bool   IsCue() const { return fCue; }

private:

	double direction;	// Направление движения шара (угол в радианах 
	// относительно оси OX)
	double energy;		// Энергия шара
	bool   fCue;		// Признак белого шара
	RedGreenBlue rgb = RedGreenBlue(0, 0, 255);
};



// Класс "Стенка бильярдного стола"
class Wall : public GraphicalObject
{
public:
	Wall(int left, int top, int right, int bottom,
		double cf);

	// Рисование стенки
	void Draw() const;
	// Извещение стенки о том, что в нее попал шар 
	void HitBy(Ball& ball);

private:

	double convertFactor;	// Значение, из которого вычитается 
	// направление ударившего шара, чтобы
	// получилось зеркальное отражение
private:
	// Запрет копирующего конструктора и оператора присваивания 
	// посредством объявления в секции private
	Wall(const Wall&);
	Wall& operator=(const Wall&);
};



// Класс "Луза бильярдного стола"  
class Hole : public GraphicalObject
{
public:
	Hole(int x, int y);

	// Запрет копирующего конструктора и операции присваивания 
	// с помощью ключевого слова delete
	Hole(const Hole&) = delete;
	Hole& operator=(const Hole&) = delete;

	// Рисование лузы
	void Draw() const;
	// Извещение лузы о том, что в нее попал шар 
	void HitBy(Ball& ball);

};



// ********************************************************************************
//   Реализация класса CRect
// ********************************************************************************

void Rect::NormalizeRect()
{
	int nx1 = x1, ny1 = y1, nx2 = x2, ny2 = y2;
	x1 = min(nx1, nx2);
	y1 = min(ny1, ny2);
	x2 = max(nx1, nx2);
	y2 = max(ny1, ny2);
}


Rect Rect::IntersectRect(const Rect& another) const
{
	// Вспомогательные переменные: копии прямоугольников для расчета 
	// пересечения - эти копии нормализуются. Исходные прямоугольники,
	// возможно, ненормализованные, не изменяются.
	Rect r1 = *this, r2 = another;
	r1.NormalizeRect();
	r2.NormalizeRect();

	// Проверка на наличие пересечения
	if ((r1.x1 >= r2.x1 && r1.x1 <= r2.x2) ||
		(r1.x2 >= r2.x1 && r1.x2 <= r2.x2))
		if ((r1.y1 >= r2.y1 && r1.y1 <= r2.y2) ||
			(r1.y2 >= r2.y1 && r1.y2 <= r2.y2))
		{
			int ix1 = max(r1.x1, r2.x1);
			int iy1 = max(r1.y1, r2.y1);
			int ix2 = min(r1.x2, r2.x2);
			int iy2 = min(r1.y2, r2.y2);
			return Rect(ix1, iy1, ix2, iy2);
		}

	// Если пересечения нет, то возвращается пустой прямоугольник
	return Rect(0, 0, 0, 0);
}



// ********************************************************************************
//   Реализация класса Wall
// ********************************************************************************

Wall::Wall(int left, int top, int right, int bottom,
	double cf) :
	GraphicalObject(left, top, right, bottom),
	convertFactor(cf) {}


void Wall::Draw() const
{
	// Стенка рисуется в виде зеленого прямоугольника
	glColor3ub(0, 255, 0);
	glRecti(region.x1, region.y1, region.x2, region.y2);
}


void Wall::HitBy(Ball& ball)
{
	// Зеркальное отражение шара от стенки 
	ball.SetDirection(convertFactor - ball.GetDirection());
}



// ********************************************************************************
//   Реализация класса Hole
// ********************************************************************************

Hole::Hole(int x, int y) :
	GraphicalObject(x - 5, y - 5, x + 5, y + 5) {}


void Hole::Draw() const
{
	// Луза рисуется в виде желтого круга, вписанного в область region
	glColor3ub(255, 255, 0);
	glPointSize(static_cast<float>(region.Width()));
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	int cx = 0, cy = 0;
	region.CenterPoint(cx, cy);
	glVertex2i(cx, cy);
	glEnd();
	glDisable(GL_POINT_SMOOTH);
}


void Hole::HitBy(Ball& ball)
{
	// Энергия шара обнуляется
	ball.SetEnergy(0.0);

	// Белый шар снова помещается в начальную позицию, а синие шары 
	// удаляются со стола и рисуются в строке над изображением стола
	if (ball.IsCue())
		ball.SetCenter(50, 108);
	else
	{
		ball.SetCenter(10 + saveRack * 15, 250);
		saveRack++;	// Увеличение глобального счетчика забитых шаров
	}
}



// ********************************************************************************
//   Реализация класса Ball
// ********************************************************************************

Ball::Ball(int x, int y, bool fc, RedGreenBlue rgb_) :
	GraphicalObject(x - 5, y - 5, x + 5, y + 5),
	fCue(fc),
	rgb(rgb_)
{
	SetCenter(x, y);
	SetDirection(0);
	SetEnergy(0.0);
}

void Ball::Draw() const
{
	// Шар рисуется вписанным в область region либо белым, либо синим цветом.
	// Белым цветом рисуется шар, у которого установлен флаг fCue. 

	if (IsCue())
		glColor3ub(255, 255, 255);	// Белый цвет
	else
		glColor3ub(rgb.GetRed(), rgb.GetGreen(), rgb.GetBlue()); // другой цвет

	glPointSize(static_cast<float>(region.Width()));
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	int cx = 0, cy = 0;
	region.CenterPoint(cx, cy);
	glVertex2i(cx, cy);
	glEnd();
	glDisable(GL_POINT_SMOOTH);
}


void Ball::SetCenter(int newx, int newy)
{
	// Задание описывающего прямоугольника для шара с центром в точке (newx, newy)
	region.SetRect(newx - 5, newy - 5, newx + 5, newy + 5);
}


void Ball::GetCenter(int& x, int& y) const
{
	region.CenterPoint(x, y);
}


void Ball::Update()
{
	// Для движения у шара должна быть некоторая кинетическая энергия
	if (energy <= 0.5)
		return;

	fBallMoved = true;

	// На каждом шаге энергия шара уменьшается за счет трения
	energy -= 0.05;

	// Смещение шара вычисляется с учетом квадратичной зависимости
	// кинетической энергии от скорости. Константа 2.0 выбрана для 
	// обеспечения более-менее реалистичного движения шаров
	int dx = static_cast<int>(2.0 * sqrt(energy) * cos(direction));
	int dy = static_cast<int>(2.0 * sqrt(energy) * sin(direction));
	region.OffsetRect(dx, dy);

	for (auto& bptr : listOfObject)
	{
		if (bptr != this)
		{
			Rect is = region.IntersectRect(bptr->GetRegion());
			if (!is.IsRectEmpty())
			{
				bptr->HitBy(*this);
				break;
			}
		}
	}

}

// Расчет угла между осью OX и вектором (dx, dy)
// Функция возвращает значение угла радианах в диапазоне (0,PI) или (-PI,0)
double Ball::HitAngle(double dx, double dy) const
{
	double na = 0.0;

	if (fabs(dx) < 0.05)
		na = PI / 2;
	else
		na = atan(fabs(dy / dx));

	if (dx < 0)
		na = PI - na;
	if (dy < 0)
		na = -na;

	return na;
}



void Ball::HitBy(Ball& ball)
{
	// Уменьшаем энергию ударившего шара вдвое
	ball.SetEnergy(ball.GetEnergy() / 2.0);
	// и прибавляем ее к собственной энергии
	energy += ball.GetEnergy();

	// Расчет нового направления для текущего шара
	int cx1 = 0, cy1 = 0, cx2 = 0, cy2 = 0;
	GetCenter(cx1, cy1);
	ball.GetCenter(cx2, cy2);
	SetDirection(HitAngle(cx1 - cx2, cy1 - cy2));

	// Модификация направления ударившего шара
	double da = ball.GetDirection() - GetDirection();
	ball.SetDirection(ball.GetDirection() + da);
}


// ********************************************************************************
//   Глобальные функции, не являющиеся членами ни одного из классов
// ********************************************************************************

RedGreenBlue ChooseRedGreenBlue(int count)
{
	if (count < 5)
	{
		return RedGreenBlue(0, 0, 255);
	}
	else
	{
		if (count < 10)
		{
			return RedGreenBlue(0, 255, 0);
		}
		else
		{
			return RedGreenBlue(255, 0, 0);
		}
	}
}

// Функция для инициализации глобальных переменных.
void CreateGlobals()
{
	saveRack = 0;		// Счетчик выбитых шаров
	
	// Создание шаров. 

	// Сначала создается белый шар, а затем 15 синих. В связном
	// списке белый шар располагается последним.
	RedGreenBlue rgb = RedGreenBlue(0, 0, 255);
	int count = 0;
	listOfObject.push_back(new Ball(50, 108, true, rgb));
	pCueBall = dynamic_cast<Ball*>(listOfObject[0]);
	for (int i = 1; i <= 5; i++)
		for (int j = 1; j <= i; j++)
		{
			rgb = ChooseRedGreenBlue(count);
			listOfObject.push_back(new Ball(190 + i * 8, 100 + 16 * j - 8 * i, false, rgb));
			++count;
		}


	// Создание стенок
	listOfObject.push_back(new Wall(10, 10, 300, 15, 0.0));
	listOfObject.push_back(new Wall(10, 200, 300, 205, 0.0));
	listOfObject.push_back(new Wall(10, 10, 15, 200, PI));
	listOfObject.push_back(new Wall(300, 10, 305, 205, PI));

	// Создание луз
	listOfObject.push_back(new Hole(15, 15));
	listOfObject.push_back(new Hole(15, 200));
	listOfObject.push_back(new Hole(300, 15));
	listOfObject.push_back(new Hole(300, 200));
	listOfObject.push_back(new Hole(157, 15));
	listOfObject.push_back(new Hole(157, 200));
}



// ********************************************************************************
//   Функции обратной связи OpenGL
// ********************************************************************************

// Обработчик события "нажатие левой кнопки мыши".
// Эта функция имитирует удар кием по белому шару. Сила удара
// постоянна, направление зависит от положения указателя мыши.
void CALLBACK MouseButtonDown(AUX_EVENTREC* event)
{
	// Запоминание координат указателя мыши в правой системе координат, связанной 
	// с нижним левым углом окна (в структуре event они хранятся в левой системе 
	// координат, связанной с левым верхним углом окна)
	int mouse_x = event->data[0];
	int mouse_y = WINDOW_HEIGHT - event->data[1];

	// Белому шару передается некоторая начальная энергия
	pCueBall->SetEnergy(20.0);
	// и присваивается направление движения "на указатель мыши"
	int cx = 0, cy = 0;
	pCueBall->GetCenter(cx, cy);
	pCueBall->SetDirection(pCueBall->HitAngle(mouse_x - cx, mouse_y - cy));

	// Флаг наличия хотя бы одного движущегося шара
	fBallMoved = true;
}


// Функция для рисования сцены: бильярдного стола и шаров
void CALLBACK Display()
{
	// Очистка буфера в памяти (он используется для отрисовки сцены)
	glClear(GL_COLOR_BUFFER_BIT);

	const GraphicalObject* pObj = listOfObject[0];

	for (auto& pObj : listOfObject)
	{
		pObj->Draw();
	}


	// Обмен буферов - экранного буфера и 
	// буфера в памяти (в котором выполнялась отрисовка)
	auxSwapBuffers();
}


// Фоновая функция
void CALLBACK Idle()
{
	// Положение шаров обновляется, только если есть хотя бы один движущийся шар
	if (!fBallMoved)
		return;

	// Обновление положения шаров
	fBallMoved = false;

	GraphicalObject* bptr = listOfObject[0];

	for (auto& bptr : listOfObject)
	{
		bptr->Update();
	}


	Display();		// Отображение сцены
	Sleep(25);	// Задержка на 25 мс, чтобы шары не двигались слишком быстро
}



// ********************************************************************************
//   Главная функция программы
// ********************************************************************************

int main()
{

	CreateGlobals();

	// Параметры обсчета сцены в OpenGL: цветовой режим RGBA, двойная буферизация
	auxInitDisplayMode(AUX_DOUBLE | AUX_RGBA);

	// Создание окна вспомогательной библиотеки Glaux для вывода изображения OpenGL.
	// Размер окна - WINDOW_WIDTHхWINDOW_HEIGHT пикселей. Левый верхний угол окна
	// задается экранными координатами (10, 10).
	auxInitPosition(10, 10, WINDOW_WIDTH, WINDOW_HEIGHT);
	auxInitWindow(L"Billiard model");

	// Включение параметров OpenGL
	glClearColor(0.0, 0.0, 0.0, 0.0);		// Цвет очистки изображения (цветового буфера)
	glShadeModel(GL_FLAT);					// Отключение режима расчета освещения (для двумерной графики)

	// Регистрация функции-обработчика события - для щелчка левой кнопкой мыши
	auxMouseFunc(AUX_LEFTBUTTON, AUX_MOUSEDOWN, MouseButtonDown);

	// Фоновая функция – периодически вызывается, когда окно не получает сообщений 
	// от клавиатуры, мыши или  таймера. С ее помощью реализуется анимация. 
	auxIdleFunc(Idle);

	// Задание функции отрисовки окна. Эта функция будет вызываться всякий
	// раз, когда потребуется перерисовать окно на экране (например, когда 
	// окно будет развернуто на весь экран)
	auxMainLoop(Display);

	return 0;
}