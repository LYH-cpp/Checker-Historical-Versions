//checker v1.3.0
//BY: LYH

//--------------------------------------------------
//以下信息必须手动设置

//下面是需填写的程序名（不要带扩展名）
//请以字符串形式填写
constexpr const char
*data_maker_C_str("data_maker"),//数据生成器
*test_C_str[]{"test1","test2"};//需对拍的程序，例：{"LYH_tcl","JSY_Orz"}


//以下信息可能需要改动

constexpr int
time_limit(3000),//时间限制，单位ms
test_times(10000);//测试程序的次数

constexpr const char
*order("-std=c++14 -O2 -Wall -Wl,-stack=134217728");//编译命令，可视情况吸氧
//--------------------------------------------------

#if __cplusplus<201103
/*
g++使用的语言标准太低，
需使用更高版本的语言标准(-std=c++11 或更高)
或下载更高版本的g++(https://github.com/jmeubank/tdm-gcc/releases/download/v10.3.0-tdm64-2/tdm64-gcc-10.3.0-2.exe)
*/
#error This code must be enabled with the -std=c++11 or -std=gnu++11 compiler options.
#endif

#include <cstdio>//输入输出
#include <fstream>//文件读写
#include <future>//多进程
#include <initializer_list>//初始化列表
#include <vector>//向量（动态数组）
#include <windows.h>//system函数 & Win32 API

using std::async;

template<typename T>
constexpr inline T Min(const T&a,const T&b){//min
	return a<b?a:b;
}

constexpr int
n((sizeof test_C_str)/(sizeof*test_C_str)),//需对拍的程序数量
compile_thread_limit(Min(n+1,3)),//编译线程限制
run_thread_limit(Min(n,3));//运行线程限制

constexpr WORD
Default(MAKEWORD(7,0)),//默认颜色
AC_col(MAKEWORD(10,0)),//AC颜色
WA_col(MAKEWORD(12,0)),//WA颜色
RE_col(MAKEWORD(13,0)),//RE颜色
TLE_col(MAKEWORD(14,0)),//TLE颜色
UKE_col(MAKEWORD(9,0)),//UKE颜色
Light(MAKEWORD(15,0));//高亮颜色

enum Exit_mode{clean_data=1,clean_thread=2,need_enter=4,waiting_animation=12,show_window=16};

class String{//当初脑子瓦特，觉得std::string不能向下转化为char数组，所以自己写了一个
	static constexpr int max_size=300;
	char s[max_size];
	int len;
public:
	String(const char*s_=""){//构造
		len=0;
		while(s_[len]){
			s[len]=s_[len];
			len++;
		}
		s[len]=0;
	}
	String(const String&s_){//复制构造
		len=0;
		while(len!=s_.len){
			s[len]=s_.s[len];
			len++;
		}
		s[len]=0;
	}

	operator const char*()const{//转化为char数组
		return s;
	}

	const String&operator=(const String&s_){//赋值
		len=0;
		while(len!=s_.len){
			s[len]=s_.s[len];
			len++;
		}
		s[len]=0;
		return*this;
	}
	const String&operator=(const char*s_){
		len=0;
		while(s_[len]){
			s[len]=s_[len];
			len++;
		}
		s[len]=0;
		return*this;
	}

	String operator+(const String&b)const{//连接字符串
		String ans(*this);
		for(int i=0;i<b.len;i++)
			ans.s[ans.len++]=b.s[i];
		ans.s[ans.len]=0;
		return ans;
	}
	friend String operator+(const char*a,const String&b){
		return String(a)+b;
	}
};

namespace Ostream{//输出命名空间
	std::mutex out;
	WORD Now(Default);//当前窗口颜色
	HANDLE hconsole(GetStdHandle(STD_OUTPUT_HANDLE));//获得标准输出设备的句柄

	inline void write(const WORD&col){
		if(Now!=col)
			SetConsoleTextAttribute(hconsole,Now=col);
	}

	template<typename T>
	void unsigned_write(T&&x){
		if(x>9)
			unsigned_write(x/10);
		putchar(x%10^48);
	}

	template<typename T>
	inline void write(T x){
		if(x<0){
			putchar(45);
			x=-x;
		}
		if(x>9)
			unsigned_write(x/10);
		putchar(x%10^48);
	}

	inline void write(const char&x){
		putchar(x);
	}

	inline void write(const char*x){
		while(*x)
			putchar(*x++);
	}

	inline void write(const String&x){
		write((const char*)x);
	}
}
template<typename... Ar>
inline void write(const Ar&...x){
	using namespace Ostream;
	out.lock();//为防止多线程输出混乱，给输出加上互斥锁，保证一次只有一个线程在输出
	std::initializer_list<int>{(write(x),0)...};//但g++的编译信息还是可能与线程信息同时输出
	out.unlock();
}//快写

const String data_maker(data_maker_C_str),make_data(data_maker+".exe > .data_next.in");
String test[n],run[n],check[n-1];
int cnt,i,list_RE[n],list_TLE[n],list_UKE[n];
bool err,extra_work,cont(true);
std::atomic<int>compile_thread(compile_thread_limit),cnt_RE,cnt_TLE,cnt_UKE,now(n),finished,flag_Shutdown;
std::condition_variable cv,Call[n];
std::future<int>result[n],data_result;
std::future<void>Future_Judge[run_thread_limit],Future_Run[n],Future_Data,Judge_result;
std::promise<int>data_wait,Ret[n];
std::promise<void>main_wait;
std::mutex lock,Temp[n];
decltype(std::chrono::steady_clock::now())st[n],ed[n];

inline int Compile(const String&x){//编译程序
	if(system("g++ "+x+".cpp -o "+x+".exe "+order)){
		write(WA_col,"Failed to compile ",x,".cpp\n\n");
		compile_thread++;
		return -1;
	}
	write(Default,"Compile ",x,".cpp successfully\n");
	compile_thread++;
	return 0;
}

inline void Run(int i){
	std::unique_lock<std::mutex>l(Temp[i]);
	while(Call[i].wait(l),cont){
		st[i]=std::chrono::steady_clock::now();
		int&&tmp(system(run[i]));
		ed[i]=std::chrono::steady_clock::now();
		Ret[i].set_value(std::move(tmp));
	}
}

inline void Judge__(const int&&i){
	if(i>=n)return;
	result[i]=(Ret[i]=std::promise<int>()).get_future();
	Temp[i].lock();
	Temp[i].unlock();
	Call[i].notify_one();//运行程序
	switch(result[i].wait_for(std::chrono::milliseconds(time_limit))){//等待直到程序结束或超时
	case std::future_status::ready://程序结束
		if(result[i].get()){//进程返回值不是0
			write(test[i],": Runtime Error\n");
			list_RE[cnt_RE++]=i;
		}
		else
			write(test[i],": spent ",std::chrono::duration_cast<std::chrono::milliseconds>(ed[i]-st[i]).count()," ms.\n");
		break;
	case std::future_status::timeout://超时
		write(test[i],": Time Limit Exceeded\n");
		system("taskkill>nul 2>nul /f /im "+test[i]+".exe\n");
		list_TLE[cnt_TLE++]=i;
		break;
	default://其它情况（如进程未启动）
		write(test[i],": Unknown Error\n");
		system("taskkill>nul 2>nul /f /im "+test[i]+".exe\n");
		list_UKE[cnt_UKE++]=i;
	}
	if(now<n)Judge__(now++);
}

inline void Make_data(){
	std::unique_lock<std::mutex> l(lock);
	main_wait.set_value();
	while(cv.wait(l),cnt!=test_times){
		l.unlock();
		data_wait.set_value(system(make_data));
		if(now<n){
			extra_work=true;
			Judge__(now++);//数据已生成，参与运行
			extra_work=false;
		}
		l.lock();
	}
}

inline void Judge(){
	std::unique_lock<std::mutex> l(lock);
	while(cv.wait(l),cont){
		l.unlock();
		Judge__(now++);
		if(++finished==run_thread_limit)main_wait.set_value();
		l.lock();
	}
}

inline void Classify_Output(){//将输出分类
	std::vector<int>tp[n];
	int cnt=0,j;
	for(i=0;i<n;i++){
		for(j=0;j<cnt&&system("fc>nul 2>nul "+test[i]+".out "+test[tp[j].front()]+".out");j++);
		if(cnt==j)cnt++;
		tp[j].emplace_back(i);
	}
	write(Light);
	for(i=0;i<cnt;i++){
		if(i){
			write(Default);
			system("fc "+test[tp[i-1][0]]+".out "+test[tp[i][0]]+".out");
		}
		write(Light,"Type of Output #",i+1,':');
		for(int x:tp[i])
			write(' ',test[x]);
		write("\n\n");
	}
}

inline void Taskkill(){
	system("taskkill>nul 2>nul /f /im "+data_maker+".exe");
	for(i=0;i<n;i++)
		system("taskkill>nul 2>nul /f /im "+test[i]+".exe");
}

[[noreturn]]inline void Exit(const int&&mode){//结束Checker
	if(mode&clean_data){
		system("taskkill>nul 2>nul /f /im "+data_maker+".exe");
		system("del>nul 2>nul .data_next.in /f /q");
	}
	if(mode&clean_thread){
		cont=false;
		cnt=test_times;
		cv.notify_all();
		for(i=0;i<n;i++)
			Call[i].notify_one();
	}
	if(flag_Shutdown)
		exit(0);
	if(mode&show_window){
		HWND hWnd=GetConsoleWindow();//获取当前控制台窗口句柄
		ShowWindow(hWnd,SW_SHOWNORMAL);//显示窗口（退出最小化）
		SetWindowPos(hWnd,HWND_TOP,0,0,450,300,0);//将窗口置于顶层的指定位置
	}
	if(mode&need_enter){
		write(Default,"Please press Enter to continue");
		result[0]=async(std::launch::async,getchar);
		if((mode&waiting_animation)==waiting_animation){
			for(i=1;result->wait_for(std::chrono::seconds(1))!=std::future_status::ready;i=(i+1)&3)//等待程序结束
				if(i)
					write('.');
				else{
					i=1;
					write("\b\b\b.  \b\b");
				}
		}
		else{
			write("...");
			result->get();
		}
	}
	exit(0);
}

inline void Add_bat(){//添加批处理
	std::ofstream out;
	out.open(".Compile_and_Run_Checker.bat");//编译和运行checker
	out<<(const char*)(String("g++ \"")+__FILE__+"\" -o \""+__FILE__);
	out.seekp(-3,std::ios::cur);
	out<<(const char*)(String("exe\" -Ofast -Wall -Werror\n@if %errorlevel%==0 (\n\"")+__FILE__);
	out.seekp(-3,std::ios::cur);
	out<<"exe\"\n) else (\npause\n)";
	out.close();
	out.open(".Erase_Output_Files.bat");//删除输出文件
	out<<"del *.out /f /q";
	out.close();
	out.open(".Erase_Exe_Files.bat");//删除可执行文件
	out<<"del *.exe /f /q";
	out.close();
	out.open(".Help.bat");//获取帮助
	out<<"start \"\" \"https://qoj.fzoi.top/post/4015\"\nstart \"\" \"https://github.com/LYH-cpp/Checker/issues\"";
	out.close();
}

int main(int argc,char**argv){
	system("title LYH's Checker");
	system("cls");
	SetWindowPos(GetConsoleWindow(),HWND_TOP,0,0,450,300,SWP_NOMOVE);
	if(system("g++>nul 2>nul -v")){
		/*
		未配置编译器，需将g++的文件路径添加到环境变量Path中
		在（系统属性->高级系统设置->环境变量->系统变量->Path 中）
		新建一项，加入"C:\TDM-GCC-64\bin"
		（这里因自己的编译器路径而改变，如果有Dev-C++可以写"C:\Program Files (x86)\Dev-Cpp\MinGW64\bin"）。
		*/
		write(WA_col);
		system("g++");
		write("Complie Error\nDid you download Complier?\nPlease download GCC and check your Path.\n");
		Exit(waiting_animation|show_window);
	}
	for(i=0;i<n;i++)test[i]=test_C_str[i];
	if(!SetConsoleCtrlHandler((PHANDLER_ROUTINE)[](DWORD cevent){flag_Shutdown=1;Taskkill();return TRUE;},true))
		write(WA_col,"Could not set control handler\n");
	Future_Judge[0]=async(std::launch::async,Add_bat);
	Taskkill();
	system("del>nul 2>nul !data.in /f /q");
	Future_Judge->wait();
	write(Light,"Compiling...\n");
	for(i=0;i<n;i++){
		compile_thread--;
		write(Light,"Start to compile ",test[i],".cpp\n");
		result[i]=async(std::launch::async,Compile,test[i]);//启动一个新的编译线程
		while(compile_thread<1)//当前进程数量达到上限，等待
			Sleep(1);
	}
	write(Light,"Start to compile ",data_maker,".cpp\n");
	if((err=Compile(data_maker))){//编译数据生成器
		write(Light,"Retry to compile ",data_maker,".cpp\n");
		err=Compile(data_maker);
	}
	for(i=0;i<n;i++)
		if(result[i].get()&&!err){
			write(Light,"Retry to compile ",test[i],".cpp\n");
			err=Compile(test[i]);
		}
	if(err){
		write(WA_col,"Compilation failed\n");
		Exit(need_enter|show_window);
	}
	write(AC_col,"Compilation succeed\n");
	Judge_result=main_wait.get_future();
	Future_Data=async(std::launch::async,Make_data);//生成数据
	data_result=data_wait.get_future();
	Judge_result.get();
	lock.lock();
	lock.unlock();
	cv.notify_one();
	for(i=0;i<run_thread_limit;i++)
		Future_Judge[i]=async(std::launch::async,Judge);
	for(i=0;i<n;i++){
		Future_Run[i]=async(std::launch::async,Run,i);
		run[i]=test[i]+".exe < !data.in > "+test[i]+".out";//运行用字符串
	}
	for(i=0;i<n-1;i++)
		check[i]="fc>nul 2>nul "+test[i]+".out "+test[i+1]+".out";//比较输出用字符串
	while(cnt++!=test_times){
		write("\x1B[2J\x1B[H",Light,"Test#",cnt,":\nMaking data");
		if(!extra_work){
			for(i=1;data_result.wait_for(std::chrono::seconds(1))!=std::future_status::ready;i=(i+1)&3)//等待数据生成
				if(i)
					write('.');
				else{
					i=1;
					write("\b\b\b.  \b\b");
				}
		}
		if(data_result.get()){//数据生成器RE了
			write(WA_col,"\b\b\b\b\b\b\b\b\b\b\b\b\b\bMake data unsuccessfully\n");
			Exit(clean_data|clean_thread|waiting_animation|show_window);
		}
		write("\b\b\b\b\b\b\b\b\b\b\b\b\b\bMake data successfully\n");
		system("ren .data_next.in !data.in");
		now=0;
		finished=0;
		Judge_result=(main_wait=decltype(main_wait)()).get_future();
		data_result=(data_wait=decltype(data_wait)()).get_future();
		cv.notify_all();
		Judge_result.get();
		while(extra_work)
			Sleep(1);
		if(cnt_RE||cnt_TLE||cnt_UKE){//输出信息
			write(WA_col,"\nUnaccepted on test#",cnt,'\n');
			if(cnt_RE){
				write(RE_col,"Runtime Error: ",Light);
				for(i=0;i<cnt_RE;i++)
					write(test[list_RE[i]],' ');
				write('\n');
			}
			if(cnt_TLE){
				write(TLE_col,"Time Limit Exceeded: ",Light);
				for(i=0;i<cnt_TLE;i++)
					write(test[list_TLE[i]],' ');
				write('\n');
			}
			if(cnt_UKE){
				write(UKE_col,"Unknown Error: ",Light);
				for(i=0;i<cnt_UKE;i++)
					write(test[list_UKE[i]],' ');
				write('\n');
			}
			Exit(clean_data|clean_thread|waiting_animation|show_window);
		}
		for(i=0;i<n-1&&!system(check[i]);i++);//检查输出
		if(i<n-1){//Wrong Answer
			write(WA_col,"\nUnaccepted on test#",cnt,"\nWrong Answer!\n\n");
			if(n>2)Classify_Output();
			else system("fc "+*test+".out "+test[1]+".out");
			Exit(clean_data|clean_thread|waiting_animation|show_window);
		}
		write(AC_col,"\nAccepted!");
		system("del>nul 2>nul !data.in /f /q");
		// Sleep(500);
	}
	system("del>nul 2>nul !data.in .data_next.in "+data_maker+".exe *.out /f /q");
	for(i=0;i<n;i++)
		system("del>nul 2>nul "+test[i]+".exe /f /q");
	write("\x1B[2J\x1B[H");//清空屏幕
	write(Light,"Tested ",test_times," times\n",AC_col,"All Accepted!\n");
	Exit(clean_thread|waiting_animation|show_window);
}