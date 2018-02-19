//#include <stdio.h>
#include <stdlib.h>
#include <complex>
#include <iostream>


using namespace std;

#define N 34  //node total number
#define BRN 33 //Branch total number
#define eps 0.000001


complex<double> S[N];//各节点注入功率
complex<double> V[N];//节点电压
complex<double> Il[BRN];//支路电流
complex<double> In[N]; //各节点注入电流失量
int lineno[BRN];       //支路号

//光伏电源相关参数
int pvnode;//光伏接入节点号 
double Pi=150;//kW 恒定光伏输出有功
double Ipv=50;//A  恒定光伏电流


//节点结构体
typedef struct Node
{
	int no;  //节点编号
    double P,Q; //节点功率注入（负荷功率）
}Node;


//支路结构体
typedef struct 
{
    int from, to;//首段 末端节点
    double r,x;//支路阻抗
}Branch;


//邻接链表节点类型
typedef struct adjv
{
	int adjno;//邻接节点号
	struct adjv* next; //下一个邻接节点
}adjvex;

   
 
  
int main()
{
	
	int i,j,k;
	Node node[N];
	Branch brn[BRN];//对辐射状电网 支路数=节点数-1;BRN=N-1

	adjvex* adjlist[N]; //定义邻接数组 存储n个表头指针

	//读数据
	FILE * fp;
	if((fp=fopen("solor.txt","rb"))==NULL)
	{
		printf("File open failed!");
		exit(1);
	}
    

 
	//读数据
	for(i=0;i<BRN;i++)
	{
		fscanf(fp,"%d %d %d %lf %lf %lf %lf", 
			   &lineno[i],&brn[i].from,&brn[i].to, 
			   &brn[i].r,&brn[i].x,                 //支路数据
			   &node[i+1].P, &node[i+1].Q //节点数据
			   );
		node[brn[i].to].no=brn[i].to;
		S[brn[i].to]=complex<double> (node[i+1].P,node[i+1].Q); 
		
	}
	node[0].no=0;node[0].P=0;node[0].Q=0;//根节点信息
    S[0]=complex<double>(node[0].P,node[0].Q);

    printf("\n请指明光伏接入母线号:\n");
	scanf("%d",&pvnode);
	

	//遍历支路形成邻接链表
	for(i=0;i<N;i++)
		adjlist[i]=(adjvex*)malloc(sizeof(adjvex));

	for(i=0;i<N;i++)
	{
		adjlist[i]->next=NULL;
		adjlist[i]->adjno=i;
		//printf("%d:\n",adjlist[i]->adjno);
	}

    
	for(k=0;k<BRN;k++)
	{
		//从根节点开始
		i=brn[k].from;
		j=brn[k].to;
		//printf("i=%d \n", i);
		adjvex* p=new adjvex;
        p->adjno=j;
		//将新节点插入到Vi邻接点表头
		p->next=adjlist[i]->next;
		adjlist[i]->next=p; 

	}

	/*调试用 *for(i=0;i<N;i++)
	{
		printf("节点%d的邻接节点为:\n",adjlist[i]->adjno);
		adjvex * p=adjlist[i]->next;

		while(p!=NULL){
			printf("%d\n",p->adjno);
			p=p->next;
		printf("\n");
	}*/
	
 
    //广度优先搜索得到节点计算顺序
	int order[N];
    //int visited[N]; //此处 对树状网（辐射状），可以不用标志位
	//for(i=0;i<N;i++) visited[i]=0; //访问标记初始化

	i=0;
    int *q=new int[N];
	int front=0,rear=0; //定义一个队列存放当前已被访问，但其邻接点未被访问的结点
	int cnt=0;
	//visited[i]=1;	 //
	//入队
	q[rear]=i;
	rear=(rear+1)%N;//用的是循环队列
	order[cnt++]=i; //此处的处理即：记录于节点计算顺序数组order中
	while(front!=rear) //队不为空
	{
		//删除队首元素(出队)
		int kk=q[front];
		front=(front+1)%N;
		adjvex* p=adjlist[kk]->next; //取节点k邻接表的表头指针
		while(p!=NULL)
		{//依次搜索节点k的每一个邻接点
			int j=p->adjno;//vj为vk的一个邻接点
			//if(!visited[j]) //若j没被访问过 则进行相应处理
			//{
				//此处的处理即：记录于节点计算顺序数组order中
				order[cnt++]=j;

				//visited[j]=1;//标记已访问过
				//入队
				q[rear]=j;
				rear=(rear+1)%N;
			//}
			p=p->next;
		}
	}
	//cnt--;

	printf("\n");
	for(i=0;i<N;i++)
		printf("%d\n", order[i]);


	//各节点电压初始化
	V[0]=complex<double> (10,0);//根节点电压

	//其它节点电压初值
	for(i=1;i<N;i++)
		V[i]=complex<double> (10,0);

    int t;//计算支路电流时用的支路号
	int iternum=0;//迭代次数
	double dvmax=0.1;//电压最大失配赋初值以启动迭代
	complex<double> V2;//保存上轮电压旧值
	

    while(dvmax>eps)  //电压失配量满足精度否
    {
		iternum++;//迭代次加一

		/*--Step 1：各负荷节点节点注入电流更新--*/
		cout<<"In:"<<endl;
		for(i=1;i<=N-1;i++)
		{
			//先处理光伏节点
			 if(i==pvnode) 
			 {
				//限定输出的逆变器模拟，分为电流控制型和电压控制型。电流控制
				//时可当作PI节点 若为电压控制型，则处理为PV发电机节点，输入电流达到边界值后
				//转化为PI节点如上处理。(凡储能系统可以作为PI节点处理)

				// 1)光伏电源给定有功Pi
				node[i].P=Pi;
			
				// 2)光伏电源恒定输出电流
				In[i]=Ipv;     //!此处可以将实数直接赋给复数类型 经过检验! 虚部自动置零
				
				// 3) 更新光伏电源的无功
			    node[i].Q=sqrt(Ipv*Ipv*norm(V[i])-Pi*Pi);

				// 4) 视为PQ节点参与常规潮流计算
				S[i]=complex<double> (-node[i].P, -node[i].Q); //注意这里是发电，所以有功带负号（视为“ 负的”负荷）
				cout<<S[i]<<endl;

			 }
			
			In[i]=conj(S[i]/V[i]);
			cout<<In[i]<<endl;
		}	

		    In[0]=complex<double>(0,0);//根节点
		
		
	    /*--Step 2：回代求各支路电流--*/ 
		/*根据节点功率和节点电压初值得到各负荷节点的注入电流后，按
		  order队列中从后向前的顺序计算得到各条支路上的电流。

		  处理每个节点时，分别搜索以该节点作为末节点和首节点的支路。

		  每个节点只存在一条其作为末节点的支路，这是由辐射型配
	      电网的特殊结构所决定的。*/
          for(i=N-1;i>0;i--)  //根节点0不用搜,直接搜N-1个（对辐射网即BRN）非根节点（负荷节点）
		  {                   //从而求出BRN（即N-1）个支路的电流
			  for(j=0;j<BRN;j++) //搜索以该节点作为末节点的支路
			  {
				  if(order[i]==brn[j].to)
			  		  break;
			  }
			  t=j;
			  Il[t]=In[order[i]];//计入该线路末节点的负荷电流
			  
			  //搜索以该节点作为首节点的支路
			  for(j=0;j<BRN;j++)
			  {
				 if(order[i]==brn[j].from) //有，则累加该支路的电流
			  	 Il[t]+=Il[j];	  //Il数组与brn数组一一对应
			  }

		  }
		  cout<<endl<<"Il:"<<endl;
		  for(i=0;i<BRN;i++)
		  {
			  
			 cout<<Il[i]<<endl;
		  }

		dvmax=0; //本轮电压最大失配初始化
        /*--Step 3：前推求各节点电压--*/
		for(i=1;i<N;i++)   //按order数组正序计算各负荷节点（即非根节点）的电压
		{
			//cout<<endl<<"V2="<<V[order[i]]<<endl;
			//cout<<abs(V[order[i]])<<endl;

			V2=V[order[i]];
			for(j=0;j<BRN;j++)     //处理每个节点时，搜索其作为末节点的支路，根据该支路首节点电压
			{	                   //和支路阻抗可得到该节点电压
				if(order[i]==brn[j].to)
					break;
			}
			t=j;
			//调试cout<<endl<<order[i]<<"　"<<"V["<<brn[t].from<<"]="<<V[brn[t].from]
				// <<"Il["<<t<<"]="<<Il[t]<<endl;
			V[order[i]]=V[brn[t].from]-Il[t]*(complex<double> (brn[t].r/1000,brn[t].x/1000));

			//cout<<endl<<"V="<<V[order[i]]<<endl;
			//cout<<abs(V[order[i]])<<endl;

			//printf("\nfabs(abs(V2)-abs(V[order[i]]))=%lf\n", fabs(abs(V2)-abs(V[order[i]])));	
			if( fabs(abs(V2)-abs(V[order[i]])) > dvmax ) //计算本轮前推中的最大电压失配量
				dvmax=fabs(abs(V2)-abs(V[order[i]]));
		}

		//printf("\ndvmax= %lf\n", dvmax);	
		cout<<endl<<"V"<<endl;
		for(i=0;i<N;i++)
	    	 cout<<V[i]<<endl;
		
    }//while
	
	//结果输出
	if((fp=fopen("output.txt","w"))==NULL)
	{
		printf("File open failed!");
		exit(1);
	}
	fprintf(fp,"--节点号----节点电压幅值----节点电压相角----\r\n");
	//cout<<endl<<"V"<<endl;
	for(i=0;i<N;i++)
	{
		//V[i]=V[i]/(complex<double>(10,0));
		//cout<<V[i]<<" "<<abs(V[i])<<" "<<arg(V[i])<<endl;
		fprintf(fp,"   %-2d         %-7f        %f\r\n",i,abs(V[i])/10,arg(V[i]));
	    
	}
	cout<<endl<<"迭代次数"<<iternum<<endl;
	
	return 0;
}

