//checker_v1.1.4
#include <cstdio>//输入输出
#include <windows.h>//system 函数
#include <future>//多进程
#include <mutex>//互斥锁（多进程用）
class String{//当初脑子瓦特，觉得std::string不能向下转化为char数组，所以自己写了一个
	static constexpr int max_size=1000;
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
	operator char*(){//转化为char数组
		return s;
	}
	const String&operator=(const String&s_){//赋值
		len=0;
		while(len!=s_.len){
			s[len]=s_.s[len];
			len++;
		}
		s[len]=0;
		return *this;
	}
	const String&operator=(const char*s_){
		len=0;
		while(s_[len]){
			s[len]=s_[len];
			len++;
		}
		s[len]=0;
		return *this;
	}
	String operator+(const String&b)const{//连接字符串
		String ans=*this;
		for(int i=0;i<b.len;i++)ans.s[ans.len++]=b.s[i];
		ans.s[ans.len]=0;
		return ans;
	}
	friend String operator+(const char*a,const String&b){
		return String(a)+b;
	}
	void write()const{
		const char*x=s;
		while(*x)putchar(*x++);
	}
};
namespace IO{//输入输出命名空间
	template<typename T>
	void unsigned_write(T&&x){
		if(x>9)unsigned_write(x/10);
		putchar(x%10^48);
	}
	template<typename T>
	inline void write(T x){
		if(x<0){
			putchar(45);
			x=-x;
		}
		if(x>9)unsigned_write(x/10);
		putchar(x%10^48);
	}
	inline void write(const char&x){putchar(x);}
	inline void write(const char*x){while(*x)putchar(*x++);}
	inline void write(const String&x){x.write();}
}
std::mutex out;
template<typename... Ar>
inline void write(const Ar&...x){
	out.lock();//为防止多线程输出混乱，给输出加上互斥锁，保证一次只有一个线程在输出
	int a[]{(IO::write(x),0)...};//但g++的编译信息还是可能与线程信息同时输出
	out.unlock();
}//快写
constexpr int 
n=2,//需对拍的程序数量
time_limit=3000,//时间限制
compile_thread_limit=5;//编译线程限制

//下面是需填写的程序名（不要带扩展名）
//请以字符串形式填写
const String
data_maker="data_maker",//数据生成器
test[n]={"test1","test2"},//需对拍的程序
order="-std=c++14 -Wall -Wl,-stack=134217728";//编译命令

String make_data=data_maker+".exe > .data.in",run[n],check[n];
int cnt,i,ERR;
enum Error{CE=1,RE=2,TLE=4,UKE=8};
std::atomic<int>compile_thread(compile_thread_limit);
std::future<int> result[n],data_result;
inline int Compile(const String&x){//编译程序
	if(system("g++ "+x+".cpp -o "+x+".exe "+order)){
		write("Failed to compile ",x,".cpp \n");
		compile_thread++;
		return -1;
	}
	write("Compile ",x,".cpp successfully\n");
	compile_thread++;
	return 0;
}
inline int Run(const int&i){
	return system(run[i]);
}
inline void Exit(){
	HWND hWnd=GetConsoleWindow();//获取当前控制台窗口句柄
	ShowWindow(hWnd,SW_SHOWNORMAL);//显示窗口（退出最小化）
	SetWindowPos(hWnd,HWND_TOP,0,0,800,450,0);//将窗口置于顶层的指定位置
	write("Please press Enter to continue...");
	getchar();
	exit(0);
}
int main(){
	write("Compiling...\n");
	for(i=0;i<n;i++){
		compile_thread--;
		result[i]=async(std::launch::async,Compile,test[i]);//启动一个新的编译线程
		while(compile_thread<1);//当前进程数量达到上限，等待
	}
	data_result=std::async(std::launch::async,Compile,data_maker);//编译数据生成器
	for(i=0;i<n;i++)
		if(result[i].get())
			ERR=Error::CE;
	if(ERR||data_result.get()){
		write("Compilation failed\n");
		return 0;
	}
	write("Compilation completed\n");
	for(i=0;i<n-1;i++){
		run[i]=test[i]+".exe < .data.in > "+test[i]+".out";//运行用字符串
		check[i]="fc "+test[i]+".out "+test[i+1]+".out";//检查用字符串
	}
	run[i]=test[i]+".exe < .data.in > "+test[i]+".out";
	while(true){
		cnt++;
		system("cls");//清空屏幕
		write("Test#",cnt,":\n");
		system(make_data);//生成数据
		for(i=0;i<n;i++){
			result[i]=std::async(std::launch::async,Run,i);//运行程序
			switch(result[i].wait_for(std::chrono::milliseconds(time_limit))){//等待直到程序结束或超时
				case std::future_status::ready://程序结束
					if(result[i].get()){//进程返回值不是0
						write(test[i],".exe: Runtime Error\n");
						ERR|=Error::RE;
					}
					break;
				case std::future_status::timeout://超时
					write(test[i],".exe: Time Limit Exceeded\n");
					system("taskkill /f /im "+test[i]+".exe\n");
					ERR|=Error::TLE;
					break;
				default://其它情况（如进程未启动）
					write(test[i],".exe Unknown Error\n");
					system("taskkill /f /im "+test[i]+".exe\n");
					ERR|=Error::UKE;
			}
		}
		if(ERR){
			if(ERR&Error::RE)write("Runtime Error!\n");
			if(ERR&Error::TLE)write("Time Limit Exceeded!\n");
			if(ERR&Error::UKE)write("Unknown Error!\n");
			Exit();
		}
		for(i=0;i<n-1;i++)
			if(system(check[i])){//检查
				write("Wrong Answer!\n");
				Exit();
			}
		write("Accepted!\n");
	}
	return 1;
}