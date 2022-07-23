// aircraft war2.0.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//飞机大战
#include<iostream>
#include<graphics.h>
#include<vector>
#include<conio.h>
using namespace std;

constexpr auto swidth = 600;//游戏界面宽高
constexpr auto sheight = 700;
constexpr unsigned int SHP = 3;//我方飞机最大血量
constexpr auto hurtTime = 1000;//受伤冷却时间
//判断鼠标是否点击按钮
bool PointInRect(int x, int y, RECT& r)
{
	return (r.left <= x && x <= r.right && r.top <= y && y <= r.bottom);
}
//判断两个物体是否碰撞
bool RectCrashRect(RECT &r1,RECT &r2)
{
	RECT r;
	r.left = r1.left - (r2.right - r2.left);
	r.right = r1.right;
	r.top = r1.top - (r2.bottom - r2.top);
	r.bottom = r1.bottom;

	return (r.left <= r2.left && r2.left <= r.right && r.top <= r2.top && r2.top <= r.bottom);
}

//开始界面
void Welcome()
{
	LPCTSTR title = _T("飞机大战");
	LPCTSTR tplay = _T("开始游戏");
	LPCTSTR texit = _T("退出游戏");

	RECT tplayr{}, texitr{};
	BeginBatchDraw();	//绘制飞机大战标题
	setbkcolor(WHITE);
	cleardevice();
	settextstyle(60, 0, _T("黑体"));
	settextcolor(BLACK);
	outtextxy(swidth / 2 - textwidth(title) / 2, sheight / 5, title);

	settextstyle(40, 0, _T("黑体"));	//绘制开始和退出游戏
	tplayr.left = swidth / 2 - textwidth(tplay) / 2;	//设置开始游戏框位置
	tplayr.right = tplayr.left + textwidth(tplay);
	tplayr.top = sheight / 5 * 2.5;
	tplayr.bottom = tplayr.top + textheight(tplay);

	texitr.left = swidth / 2 - textwidth(texit) / 2;
	texitr.right = texitr.left + textwidth(texit);
	texitr.top = sheight / 5 * 3;
	texitr.bottom = texitr.top + textheight(texit);

	outtextxy(tplayr.left, tplayr.top, tplay);
	outtextxy(texitr.left, texitr.top, texit);

	EndBatchDraw();
//检测玩家鼠标输入信息
	while (true)
	{
		ExMessage mess;
		getmessage(&mess, EM_MOUSE);
		if (mess.lbutton)
		{
			if (PointInRect(mess.x, mess.y, tplayr))//开始游戏
			{
				return;
			}
			else if (PointInRect(mess.x, mess.y, texitr))//离开游戏
			{
				exit(0);
			}
		}
	}

}
//游戏结束界面
void Over(unsigned long long &kill)
{
	TCHAR* str = new TCHAR[128];
	_stprintf_s(str, 128, _T("击杀数：%llu"), kill);

	settextcolor(RED);
	outtextxy(swidth / 2 - textwidth(str) / 2, sheight / 5, str);

	//键盘事件（按enter返回）
	LPCTSTR info = _T("按Enter返回");
	settextstyle(20, 0, _T("黑体"));
	outtextxy(swidth - textwidth(info), sheight - textheight(info), info);

	while (true)
	{
		ExMessage mess;
		getmessage(&mess,EM_KEY);
		if (mess.vkcode == 0x0D)
		{
			return;
		}
	}
}

//局内背景
class BK
{
public:
	BK(IMAGE& img)
		: img(img),y(-sheight),x(0)
	{

	}
//背景图往下移动，当移动完后复位重复往下移动
	void Show()
	{
		if (y == 0)
		{
			y = -sheight;
		}
		y += 7;
		putimage(0, y, &img);
	}

private:
	IMAGE& img;
	int x, y;
};
//我方战机
class Hero
{
public:
	Hero(IMAGE& img)
		:img(img), HP(SHP)
	{
		//初始化最初位置
		rect.left = swidth / 2 - img.getwidth() / 2;
		rect.top = sheight - img.getheight();
		rect.right = rect.left + img.getwidth();
		rect.bottom = sheight;
	}

	void Show()
	{
		
		putimage(rect.left, rect.top, &img);
		//展示血条
		if (HP > 0) 
		{
			setlinecolor(RED);
			setlinestyle(PS_SOLID,5);
			line(rect.left, rect.top-3, rect.left+(img.getwidth() / SHP * HP), rect.top-3);
		}
		
		std::cout <<"Hero HP="<< HP << endl;
	}

	void Control()
	{
		//玩家控制飞机
		ExMessage mess;
		if (peekmessage(&mess, EM_MOUSE))
		{
			rect.left = mess.x - img.getwidth() / 2;
			rect.top = mess.y - img.getheight() / 2;
			rect.right = rect.left + img.getwidth();
			rect.bottom = rect.top + img.getheight();
		}
	}

	bool Hurt()
	{
		//飞机受伤判定
		HP -= 1;
		return (HP == 0) ? false : true;
//		return true;
	}
	bool Alive()
	{
		return HP == 0 ? false : true;
	}

	RECT& GetRect() { return rect; }

private:
	IMAGE& img;
	RECT rect;

	unsigned int HP;
};
//敌军
class Enemy
{
public:
	Enemy(IMAGE& img,float x)
		:img(img)
	{
		rect.left = x;
		rect.right = rect.left + img.getwidth();
		rect.top = -img.getheight();
		rect.bottom = 0;
	}
	bool Show()
	{
		if (rect.top >= sheight)
		{
			return false;
		}
		rect.top += 4;
		rect.bottom += 4;
		putimage(rect.left, rect.top, &img);
		return true;
	}

	RECT& GetRect() { return rect; }

private:
	IMAGE& img;
	RECT rect;
};
//子弹
class Bullet
{
public:
	Bullet(IMAGE& img,RECT pr)
		:img(img)
	{
		rect.left = pr.left + (pr.right - pr.left) / 2 - img.getwidth() / 2;
		rect.right = rect.left + img.getwidth();
		rect.top = pr.top - img.getheight();
		rect.bottom = rect.top + img.getheight();
	}
	bool Show()
	{
		if (rect.bottom <= 0)
		{
			return false;
		}
		rect.top -= 3;
		rect.bottom -= 3;
		putimage(rect.left, rect.top, &img);
		return true;
	}
	RECT& GetRect() { return rect; }

private:
	IMAGE& img;
	RECT rect;
};
//生成敌军
bool AddEnemy(vector<Enemy*>& es, IMAGE& enemyimg)
{
	//随机生成敌军水平方向位置
	float x = abs(rand()) % (swidth - enemyimg.getwidth());
	Enemy* e = new Enemy(enemyimg, x);
	//防止生成的敌军互相碰撞
	for (auto& i : es)
	{
		if (RectCrashRect(i->GetRect(), e->GetRect()))
		{
			delete e;
			return false;
		}
	}
	es.push_back(e);
	return true;
}

bool Play()
{
	setbkcolor(WHITE);
	cleardevice();
	bool is_play = true;
	//加载本地图片
	IMAGE heroimg, enemyimg, bkimg, bulletimg;
	loadimage(&heroimg, _T("D:\\C++Project\\aircraft wars\\aircraft war2.0\\PlaneFight_BC-main\\images\\me1.png"));
	loadimage(&enemyimg, _T("D:\\C++Project\\aircraft wars\\aircraft war2.0\\PlaneFight_BC-main\\images\\enemy1.png"));
	loadimage(&bkimg, _T("D:\\C++Project\\aircraft wars\\aircraft war2.0\\PlaneFight_BC-main\\images\\bk2.png"),swidth,sheight*2);
	loadimage(&bulletimg, _T("D:\\C++Project\\aircraft wars\\aircraft war2.0\\PlaneFight_BC-main\\images\\bullet1.png"));

	BK bk = BK(bkimg);
	Hero hp = Hero(heroimg);

	vector<Enemy*>es;
	vector<Bullet*>bs;

	int bsing = 0;
	unsigned long long kill = 0;
	clock_t hurtLast = clock();

	for (int i = 0; i < 2; i++)
	{
		AddEnemy(es,enemyimg);
	}

	while (is_play)
	{
		//每30帧生成一颗子弹
		bsing += 1;
		if (bsing == 30)
		{
			bsing = 0;
			bs.push_back(new Bullet(bulletimg, hp.GetRect()));
		}

		BeginBatchDraw();

		bk.Show();
		Sleep(6);
		flushmessage();
		Sleep(2);
		hp.Control();
		//空格键暂停，再次点击恢复
		if (_kbhit())
		{
			char v = _getch();
			if (v == 0x20)
			{
				Sleep(500);
				while (true)
				{
					if (_kbhit())
					{
						v = _getch();
						if (v == 0x20)
						{
							break;
						}
					}
				}
			}
		}
		hp.Show();
		//绘制子弹
		auto bit = bs.begin();
		while (bit != bs.end())
		{
			//判定子弹是否出界
			if (!(*bit)->Show())
			{
				delete(*bit);
				bit = bs.erase(bit);
			}
			else {
				bit++;
			}
		}

		auto it = es.begin();
		while (it != es.end())
		{
			std::cout << "1" << endl;
			//判定我方是否受伤
			if (RectCrashRect((*it)->GetRect(), hp.GetRect()))
			{
				if (clock() - hurtLast >= hurtTime)
				{
					is_play = hp.Hurt();
					hp.Show();
					hurtLast = clock();
				}
				
			}
			std::cout << "2" << endl;
			//判定子弹与敌机相撞
			auto bit = bs.begin();
			while (bit != bs.end())
			{
				if (RectCrashRect((*bit)->GetRect(), (*it)->GetRect()))
				{

					delete(*it);
					it=es.erase(it);
					delete(*bit);
					bs.erase(bit);
					kill++;
					break;
				}
				bit++;
				
			}
			std::cout << "3" << endl;
			//判断es是否到尾
			if (it == es.end()) {
				break;
			}
			//绘制敌机
			if (!(*it)->Show())
			{
				delete(*it);
				it = es.erase(it);
			}
			else {
				it++;
			}
		}
		
		std::cout << "4" << endl;
		//增加敌机
		for (int i = 0; i < 5 - es.size(); i++)
		{
			AddEnemy(es,enemyimg);
		}
		std::cout << "5" << endl;
		EndBatchDraw();
	}
	Over(kill);

	return true;
}

int main()
{
	//初始化easyx
	initgraph(swidth, sheight, EW_NOMINIMIZE | EW_SHOWCONSOLE);
	bool is_live = true;
	while (is_live)
	{
		Welcome();
		is_live = Play();
	}


	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
