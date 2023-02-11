#include <cstdio>
#include <windows.h>
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

class String{
	static constexpr int max_size=1000;
	char s[max_size];
	int len;
public:
	String(const char*s_=""){
		len=0;
		while(s_[len]){
			s[len]=s_[len];
			len++;
		}
		s[len]=0;
	}
	String(const String&s_){
		len=0;
		while(len!=s_.len){
			s[len]=s_.s[len];
			len++;
		}
		s[len]=0;
	}
	operator char*(){
		return s;
	}
	const String&operator=(const String&s_){
		len=0;
		while(len!=s_.len){
			s[len]=s_.s[len];
			len++;
		}
		s[len]=0;
		return s;
	}
	const String&operator=(const char*s_){
		len=0;
		while(s_[len]){
			s[len]=s_[len];
			len++;
		}
		s[len]=0;
	}
	String operator+(const String&b)const{
		String ans=*this;
		for(int i=0;i<b.len;i++)ans.s[ans.len++]=b.s[i];
		return ans;
	}
	friend String operator+(const char*a,const String&b){
		return String(a)+b;
	}
};

//下面是需填写的程序名（不要带扩展名）
const String
data_maker="",//数据生成器
test1="",
test2="";
unsigned cnt;
int main(){
	write("Compiling...\n");
	system("g++ "+test1+".cpp -o "+test1+".exe -std=c++14");
	system("g++ "+test2+".cpp -o "+test2+".exe -std=c++14");
	system("g++ "+data_maker+".cpp -o "+data_maker+".exe -std=c++14");
	write("Compilation completed\n");
	static String 
	make_data=data_maker+".exe > .data.in",
	run1=test1+".exe < .data.in > "+test1+".out",
	run2=test2+".exe < .data.in > "+test2+".out",
	check="fc "+test1+".out "+test2+".out";
	while(true){
		cnt++;
		system(make_data);
		system(run1);
		system(run2);
		system("cls");
		write("Test#",cnt,":\n");
		if(system(check)){
			write("Wrong Answer!\n");
			system("pause");
			return 0;
		}
	}
	return 0;
}
