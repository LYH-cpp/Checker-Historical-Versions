//checker_v1.1.1
#include <cstdio>//输入输出
#include <windows.h>//system 函数
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
template<typename... Ar>
inline void write(const Ar&...x){int a[]{(write(x),0)...};}//快写

class String{//貌似std::string不能向下转化为char数组，所以自己写了一个
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
};

//需对拍的程序数量
constexpr int n=0;

//下面是需填写的程序名（不要带扩展名）
const String
data_maker="",//数据生成器
test[n]{},//需对拍的程序
order="-O2 -std=c++14 -Wall -Wl,-stack=134217728";//编译命令
unsigned cnt,i;
int main(){
	write("Compiling...\n");
	for(i=0;i<n;i++)
		system("g++ "+test[i]+".cpp -o "+test[i]+".exe "+order);
	system("g++ "+data_maker+".cpp -o "+data_maker+".exe "+order);//编译程序
	write("Compilation completed\n");
	static String make_data=data_maker+".exe > .data.in",run[n],check[n];
	for(i=0;i<n-1;i++){
		run[i]=test[i]+".exe < .data.in > "+test[i]+".out";//运行用字符串
		check[i]="fc "+test[i]+".out "+test[i+1]+".out";//检查用字符串
	}
	run[i]=test[i]+".exe < .data.in > "+test[i]+".out";
	while(true){
		cnt++;
		system(make_data);
		for(i=0;i<n;i++)system(run[i]);//运行程序
		system("cls");//清空屏幕
		write("Test#",cnt,":\n");
		for(i=0;i<n-1;i++){
			if(system(check[i])){//检查
				write("Wrong Answer!\n");
				system("pause");
				return 0;
			}
		}
	}
	return 0;
}