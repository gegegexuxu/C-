#include <stdio.h>
#include <stdlib.h>
#include <string.h>//strcmp，比较字符串是否相等 
#include <ctype.h>//isdigit,isalpha判断字符是否为数字/字母 

#define M 10000

typedef struct id {
	int stamp;  //标识符返回标记
	char name[M]; //存放标识符
	int type;   //标识符类型
	float value;//标识符的值
	int type_float; //变量数据类型
	id*next; //链表指针
}Token;
int token; //词法单元
int stamp;
char*src=NULL; //检索文本指针
int line=1; //行数，报错指示位置
float token_value;

enum {
	Id,Num,Int,Float,Write
}; //关键字
char str_int[10]="int",str_float[10]="float",str_write[10]="write";



Token*head=NULL,*rear=NULL,*pre=NULL,*current=NULL,*current1=NULL; //对存储标识符的链表的操作
int new_id=1;
int is_error=0;  //判断程序是否有错误
int is_var=1;    //判断变量格式是否正确
int is_typeint=0;


unsigned long readFile(char*filePath,char str[]); //读文件
void Initial();   //初始化函数
void ListPush(int stamp,char*name,int type,float value,int type_float);  //添加进链表
void ListPop();  //从链表中弹出一个元素
void Scaner(); //词法分析生成token
void match(); //匹配字符
void Reader(); //语法分析
float number();//识别常数值 

//后缀表达式递归求值
float factor();
float term_tail(float lvalue);
float term();
float expr_tail(float lvalue);
float expr();

int main(int argc,char*argv[]){
	char str[M];
	readFile(argv[1],str);
	if(!is_error){
		src=str;
		Initial();
		Scaner();
		while(token!='.')
			Reader();
	}
	return 0;
}
unsigned long readFile(char*filePath,char str[]){
	FILE*fp=fopen(filePath,"r");//打开文件
	if(fp==NULL){
		printf("Do not find!\n");
		exit(0);
	}
	unsigned long i;
	i=fread(str,1,M,fp);
	if(i >=M){
		printf("Overflow!\n");
		exit(1);
	}
	/*if(str[i - 1]!='.'){
		printf("Error,Please end up with \".\"\n");
		is_error=1;
	}*/
	for(int j=0;i<i;j++)
	{
		if(str[i]='.'&&!isdigit(str[i-1])&&!isdigit(str[i+1]))
			str[i+1]='\0';
	} 
	str[i]='\0';
	fclose(fp);
	return i;
}
void Initial(){
	head=(Token*)malloc(sizeof(Token));
	rear=head;
	head->next=NULL;
	head->stamp=-1;
	head->type=-1;
	head->value=-1;
	head->type_float=-1;
	ListPush(Int,str_int,Int,0,0);
	ListPush(Float,str_float,Float,0,1);
	ListPush(Write,str_write,Write,0,-1);
}
void ListPush(int stamp,char name[],int type,float value,int type_float){
	Token*temp=(Token*)malloc(sizeof(Token));
	for(int i=0; i < M; i++)
		temp->name[i]=name[i];
	temp->next=NULL;
	temp->stamp=stamp;
	temp->type=type;
	temp->value=value;
	temp->type_float=type_float;
	rear->next=temp;
	pre=rear;
	rear=temp;
	current=temp;
}
void ListPop(){
	free(rear);
	rear=pre;
	rear->next=NULL;
}
void Scaner(){
	int i,is_float=-1;
	while(token=*src){
		src++;
		//词法分析开始
		if(token=='.')
			return;
		else if(token=='\n')
			line++;
		else if(token=='+'||token=='-'||token=='*'||token=='/'
			||token=='='||token==';'||token=='('||token==')')
			return;
		//处理标识符
		else if(isalpha(token)){
			new_id=1;
			char*front=src - 1;
			while(isalpha(*src)||isdigit(*src))
				src++;
			char name1[M];
			int n=src - front;
			for(i=0; i < n; i++)
				name1[i]=*front++;
			name1[i]='\0';
			//判断标识符是否重复
			for(current=head->next; current!=NULL; current=current->next){
				if(!strcmp(current->name,name1)){
					current1=current;
					token=current->stamp;
					new_id=0;
					return;
				}
			}
			token=Id;
			if(current1->type_float==1)
				is_float=1;
			ListPush(token,name1,-1,0,is_float);
			return;
		}
		//处理整数，浮点数
		else if(isdigit(token)){
			int float_not=1;
			int num=1;
			token_value=(float)token - '0';
			while(isdigit(*src)||*src=='.'){
				if(*src=='.'){
					float_not=0;
					src++;
					continue;
				}
				if(float_not){
					token_value=token_value*10+(*src - '0');
					src++;
				}
				else {
					num=num*10;
					token_value+=(float)((*src - '0')*1.0 / num);
					src++;
				}
			}
			token=Num;
			return;
		}
		else if(token==' ')
			continue;
		else 
			return;
	}
}
void match(int tk){
	if(token=='.')
		return;
	else if(token!=tk){
		if(tk==Id){
			printf("Line %d: [Error]: Unrecongnizable Variable!\n",line);
			while(*src!=';')
				src++;
			Scaner();
		}
		else
			printf("Line %d: [Error]:Syntax Error,Unrecongnizable Symbols.There needs a \"%c\"\n",line,tk);
		is_error=1;
		return;
	}
	else
		Scaner();
}
void Reader(){
	if(token==Id){
		if(new_id){
			ListPop();
			printf("Line %d: [Error]: Undeclared Variable!\n",line);
			is_error=1;
		}
		Token*assign=current;
		match(Id);
		match('=');
		if(current->type_float==1){
			is_typeint=1;
		}
		float value;
		value=expr();
		if(((value -(int)value)==0)&&is_typeint==1){
			printf("Line %d: [Error]: Illegal Conversion!\n",line);
			is_error=1;
		}
		if(assign->type==Float)
			assign->value=value;
		else
			assign->value=(int)value;
		match(';');
	}
	else if(token==Int){
		new_id=1;
		match(Int);
		if(!new_id){
			printf("Line %d: Warning: The Variable Is Repeatedly Declared!\n",line);
			is_error=1;
		}
		current->type=Int;
		match(Id);
		match(';');
	}else if(token==Float){
		new_id=1;
		match(Float);
		if(!new_id){
			printf("Line %d: Warning: The Variable Is Repeatedly Declared!\n",line);
			is_error=1;
		}
		current->type=Float;
		match(Id);
		match(';');
	}
	else if(token==Write){
		float output;
		int output_type;
		match(Write);
		match('(');
		if(new_id){
			ListPop();
			printf("Line %d: [Error]:Undeclared Variable!\n",line);
			is_error=1;
		}
		else {
			output=current->value;
			output_type=current->type;
		}
		match(Id);
		if(!is_var)
		{
			match(Id);
		}
		match(')');
		match(';');
		if(!is_error){
			if(output_type==Int)
				printf("  %d  \n",(int)output);
			else if(output_type==Float)
				printf("  %g  \n",output);
		}

	}
	else if(token=='.')
		return;
}
float number(){
	float value=0;
	if(token==Id){
		if(new_id){
			ListPop();
			printf("Line %d: [Error]:Undeclared Variable!\n",line);
			is_error=1;
			match(Id);
			return 0;
		}
		else if(current->value==0){
			printf("Line %d: [Error]: Uninitial Variable!\n",line);
			is_error=1;
		}
		match(Id);
		return current->value;
	}
	else if(token==Num){
		match(Num);
		return token_value;
	}
	else if(token=='('){
		match('(');
		value=expr();
		match(')');
		return value;
	}
	return 0;
}

float factor(){
	//float value=0;
	if(token=='-'){
		match('-');
		return(0 - number());
	}
	return number();
}

float term_tail(float lvalue){
	//float value=0;
	if(token=='*'){
		match('*');
		return term_tail(lvalue*factor());
	}
	else if(token=='/'){
		match('/');
		float value;
		value=factor();
		if(value)
			return term_tail(lvalue / value);
		else {
			printf("Line %d: [Error]:0 Can Not Be Divisor!\n",line);
			is_error=1;
			return 0;
		}
	}
	else
		return lvalue;
}

float term(){
	float lvalue=0;
	lvalue=factor();
	return term_tail(lvalue);
}

float expr_tail(float lvalue){
	if(token=='+'){
		match('+');
		return expr_tail(lvalue+term());
	}
	else if(token=='-'){
		match('-');
		return expr_tail(lvalue-term());
	}
	else
		return lvalue;
}

float expr(){
	float lvalue=term();
	return expr_tail(lvalue);
}
