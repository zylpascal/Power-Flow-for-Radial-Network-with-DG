//#include <stdio.h>
#include <stdlib.h>
#include <complex>
#include <iostream>


using namespace std;

#define N 34  //node total number
#define BRN 33 //Branch total number
#define eps 0.000001


complex<double> S[N];//���ڵ�ע�빦��
complex<double> V[N];//�ڵ��ѹ
complex<double> Il[BRN];//֧·����
complex<double> In[N]; //���ڵ�ע�����ʧ��
int lineno[BRN];       //֧·��

//�����Դ��ز���
int pvnode;//�������ڵ�� 
double Pi=150;//kW �㶨�������й�
double Ipv=50;//A  �㶨�������


//�ڵ�ṹ��
typedef struct Node
{
	int no;  //�ڵ���
    double P,Q; //�ڵ㹦��ע�루���ɹ��ʣ�
}Node;


//֧·�ṹ��
typedef struct 
{
    int from, to;//�׶� ĩ�˽ڵ�
    double r,x;//֧·�迹
}Branch;


//�ڽ�����ڵ�����
typedef struct adjv
{
	int adjno;//�ڽӽڵ��
	struct adjv* next; //��һ���ڽӽڵ�
}adjvex;

   
 
  
int main()
{
	
	int i,j,k;
	Node node[N];
	Branch brn[BRN];//�Է���״���� ֧·��=�ڵ���-1;BRN=N-1

	adjvex* adjlist[N]; //�����ڽ����� �洢n����ͷָ��

	//������
	FILE * fp;
	if((fp=fopen("solor.txt","rb"))==NULL)
	{
		printf("File open failed!");
		exit(1);
	}
    

 
	//������
	for(i=0;i<BRN;i++)
	{
		fscanf(fp,"%d %d %d %lf %lf %lf %lf", 
			   &lineno[i],&brn[i].from,&brn[i].to, 
			   &brn[i].r,&brn[i].x,                 //֧·����
			   &node[i+1].P, &node[i+1].Q //�ڵ�����
			   );
		node[brn[i].to].no=brn[i].to;
		S[brn[i].to]=complex<double> (node[i+1].P,node[i+1].Q); 
		
	}
	node[0].no=0;node[0].P=0;node[0].Q=0;//���ڵ���Ϣ
    S[0]=complex<double>(node[0].P,node[0].Q);

    printf("\n��ָ���������ĸ�ߺ�:\n");
	scanf("%d",&pvnode);
	

	//����֧·�γ��ڽ�����
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
		//�Ӹ��ڵ㿪ʼ
		i=brn[k].from;
		j=brn[k].to;
		//printf("i=%d \n", i);
		adjvex* p=new adjvex;
        p->adjno=j;
		//���½ڵ���뵽Vi�ڽӵ��ͷ
		p->next=adjlist[i]->next;
		adjlist[i]->next=p; 

	}

	/*������ *for(i=0;i<N;i++)
	{
		printf("�ڵ�%d���ڽӽڵ�Ϊ:\n",adjlist[i]->adjno);
		adjvex * p=adjlist[i]->next;

		while(p!=NULL){
			printf("%d\n",p->adjno);
			p=p->next;
		printf("\n");
	}*/
	
 
    //������������õ��ڵ����˳��
	int order[N];
    //int visited[N]; //�˴� ����״��������״�������Բ��ñ�־λ
	//for(i=0;i<N;i++) visited[i]=0; //���ʱ�ǳ�ʼ��

	i=0;
    int *q=new int[N];
	int front=0,rear=0; //����һ�����д�ŵ�ǰ�ѱ����ʣ������ڽӵ�δ�����ʵĽ��
	int cnt=0;
	//visited[i]=1;	 //
	//���
	q[rear]=i;
	rear=(rear+1)%N;//�õ���ѭ������
	order[cnt++]=i; //�˴��Ĵ�������¼�ڽڵ����˳������order��
	while(front!=rear) //�Ӳ�Ϊ��
	{
		//ɾ������Ԫ��(����)
		int kk=q[front];
		front=(front+1)%N;
		adjvex* p=adjlist[kk]->next; //ȡ�ڵ�k�ڽӱ�ı�ͷָ��
		while(p!=NULL)
		{//���������ڵ�k��ÿһ���ڽӵ�
			int j=p->adjno;//vjΪvk��һ���ڽӵ�
			//if(!visited[j]) //��jû�����ʹ� �������Ӧ����
			//{
				//�˴��Ĵ�������¼�ڽڵ����˳������order��
				order[cnt++]=j;

				//visited[j]=1;//����ѷ��ʹ�
				//���
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


	//���ڵ��ѹ��ʼ��
	V[0]=complex<double> (10,0);//���ڵ��ѹ

	//�����ڵ��ѹ��ֵ
	for(i=1;i<N;i++)
		V[i]=complex<double> (10,0);

    int t;//����֧·����ʱ�õ�֧·��
	int iternum=0;//��������
	double dvmax=0.1;//��ѹ���ʧ�丳��ֵ����������
	complex<double> V2;//�������ֵ�ѹ��ֵ
	

    while(dvmax>eps)  //��ѹʧ�������㾫�ȷ�
    {
		iternum++;//�����μ�һ

		/*--Step 1�������ɽڵ�ڵ�ע���������--*/
		cout<<"In:"<<endl;
		for(i=1;i<=N-1;i++)
		{
			//�ȴ������ڵ�
			 if(i==pvnode) 
			 {
				//�޶�����������ģ�⣬��Ϊ���������ͺ͵�ѹ�����͡���������
				//ʱ�ɵ���PI�ڵ� ��Ϊ��ѹ�����ͣ�����ΪPV������ڵ㣬��������ﵽ�߽�ֵ��
				//ת��ΪPI�ڵ����ϴ���(������ϵͳ������ΪPI�ڵ㴦��)

				// 1)�����Դ�����й�Pi
				node[i].P=Pi;
			
				// 2)�����Դ�㶨�������
				In[i]=Ipv;     //!�˴����Խ�ʵ��ֱ�Ӹ����������� ��������! �鲿�Զ�����
				
				// 3) ���¹����Դ���޹�
			    node[i].Q=sqrt(Ipv*Ipv*norm(V[i])-Pi*Pi);

				// 4) ��ΪPQ�ڵ���볣�泱������
				S[i]=complex<double> (-node[i].P, -node[i].Q); //ע�������Ƿ��磬�����й������ţ���Ϊ�� ���ġ����ɣ�
				cout<<S[i]<<endl;

			 }
			
			In[i]=conj(S[i]/V[i]);
			cout<<In[i]<<endl;
		}	

		    In[0]=complex<double>(0,0);//���ڵ�
		
		
	    /*--Step 2���ش����֧·����--*/ 
		/*���ݽڵ㹦�ʺͽڵ��ѹ��ֵ�õ������ɽڵ��ע������󣬰�
		  order�����дӺ���ǰ��˳�����õ�����֧·�ϵĵ�����

		  ����ÿ���ڵ�ʱ���ֱ������Ըýڵ���Ϊĩ�ڵ���׽ڵ��֧·��

		  ÿ���ڵ�ֻ����һ������Ϊĩ�ڵ��֧·�������ɷ�������
	      ����������ṹ�������ġ�*/
          for(i=N-1;i>0;i--)  //���ڵ�0������,ֱ����N-1�����Է�������BRN���Ǹ��ڵ㣨���ɽڵ㣩
		  {                   //�Ӷ����BRN����N-1����֧·�ĵ���
			  for(j=0;j<BRN;j++) //�����Ըýڵ���Ϊĩ�ڵ��֧·
			  {
				  if(order[i]==brn[j].to)
			  		  break;
			  }
			  t=j;
			  Il[t]=In[order[i]];//�������·ĩ�ڵ�ĸ��ɵ���
			  
			  //�����Ըýڵ���Ϊ�׽ڵ��֧·
			  for(j=0;j<BRN;j++)
			  {
				 if(order[i]==brn[j].from) //�У����ۼӸ�֧·�ĵ���
			  	 Il[t]+=Il[j];	  //Il������brn����һһ��Ӧ
			  }

		  }
		  cout<<endl<<"Il:"<<endl;
		  for(i=0;i<BRN;i++)
		  {
			  
			 cout<<Il[i]<<endl;
		  }

		dvmax=0; //���ֵ�ѹ���ʧ���ʼ��
        /*--Step 3��ǰ������ڵ��ѹ--*/
		for(i=1;i<N;i++)   //��order���������������ɽڵ㣨���Ǹ��ڵ㣩�ĵ�ѹ
		{
			//cout<<endl<<"V2="<<V[order[i]]<<endl;
			//cout<<abs(V[order[i]])<<endl;

			V2=V[order[i]];
			for(j=0;j<BRN;j++)     //����ÿ���ڵ�ʱ����������Ϊĩ�ڵ��֧·�����ݸ�֧·�׽ڵ��ѹ
			{	                   //��֧·�迹�ɵõ��ýڵ��ѹ
				if(order[i]==brn[j].to)
					break;
			}
			t=j;
			//����cout<<endl<<order[i]<<"��"<<"V["<<brn[t].from<<"]="<<V[brn[t].from]
				// <<"Il["<<t<<"]="<<Il[t]<<endl;
			V[order[i]]=V[brn[t].from]-Il[t]*(complex<double> (brn[t].r/1000,brn[t].x/1000));

			//cout<<endl<<"V="<<V[order[i]]<<endl;
			//cout<<abs(V[order[i]])<<endl;

			//printf("\nfabs(abs(V2)-abs(V[order[i]]))=%lf\n", fabs(abs(V2)-abs(V[order[i]])));	
			if( fabs(abs(V2)-abs(V[order[i]])) > dvmax ) //���㱾��ǰ���е�����ѹʧ����
				dvmax=fabs(abs(V2)-abs(V[order[i]]));
		}

		//printf("\ndvmax= %lf\n", dvmax);	
		cout<<endl<<"V"<<endl;
		for(i=0;i<N;i++)
	    	 cout<<V[i]<<endl;
		
    }//while
	
	//������
	if((fp=fopen("output.txt","w"))==NULL)
	{
		printf("File open failed!");
		exit(1);
	}
	fprintf(fp,"--�ڵ��----�ڵ��ѹ��ֵ----�ڵ��ѹ���----\r\n");
	//cout<<endl<<"V"<<endl;
	for(i=0;i<N;i++)
	{
		//V[i]=V[i]/(complex<double>(10,0));
		//cout<<V[i]<<" "<<abs(V[i])<<" "<<arg(V[i])<<endl;
		fprintf(fp,"   %-2d         %-7f        %f\r\n",i,abs(V[i])/10,arg(V[i]));
	    
	}
	cout<<endl<<"��������"<<iternum<<endl;
	
	return 0;
}

