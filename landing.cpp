#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

IplImage* src = 0; 
IplImage* tmp = 0; 
IplImage* tmp1 = 0;
IplImage* org = 0;
void on_mouse( int event, int x, int y, int flags, void* ustc)
{
	static CvPoint pre_pt ;
	static CvPoint cur_pt ;
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);//初始化字体
	char temp[16];
	
	if( (event == CV_EVENT_LBUTTONDOWN)&&(flags) )//鼠标左键按下时
	{ 
		sprintf(temp,"(%d,%d)",x,y);//格式化字符串
		pre_pt = cvPoint(x,y);//获取当前点坐标值
		cvPutText(src,temp, pre_pt, &font, cvScalar(0,0, 0, 255));//在图像是打印字符
		cvCircle( src, pre_pt, 2,cvScalar(255,0,0,0) ,CV_FILLED, CV_AA, 0 );//在图像上画圆
		cvShowImage( "src", src );
		//cvCopy(src,tmp);//这句有没有，就是单目标和多目标的问题
	}
	else if( (event == CV_EVENT_MOUSEMOVE) && (flags & CV_EVENT_LBUTTONDOWN))
	{//鼠标移动并且鼠标左键按下
		sprintf(temp,"(%d,%d)",x,y);//格式化字符串
		cur_pt = cvPoint(x,y);//获取当前点坐标值		
		cvPutText(src,temp, cur_pt, &font, cvScalar(0,0, 0, 255));//在图像是打印字符
		cvRectangle(src, pre_pt, cur_pt, cvScalar(0,255,0,0), 2, 8, 0 );//在图像上画矩形
		cvShowImage( "src", src );
		cvCopy(tmp,src);//将img复制到临时图像tmp上，用于实时显示
	}
	else if( event == CV_EVENT_LBUTTONUP )
	{//鼠标左键弹起
		sprintf(temp,"(%d,%d)",x,y);//字体格式化
		cur_pt = cvPoint(x,y);//获取当前点坐标值		
		cvPutText(src,temp, cur_pt, &font, cvScalar(0,0, 0, 255));//在图像是打印字符
		cvCircle( src, cur_pt, 2,cvScalar(255,0,0,0) ,CV_FILLED, CV_AA, 0 );//在图像上画圆
		cvRectangle( src, pre_pt, cur_pt, cvScalar(0,255,0,0), 2, 8, 0 );//在图像上画矩形
		cvShowImage( "src", src );
 
		/******************************************************************/
		int width=abs(pre_pt.x-cur_pt.x); //两点横坐标差 
        int height=abs(pre_pt.y-cur_pt.y); //两点纵坐标差 
        if(width==0 || height==0)  
        {  //两者中有一个为零时销毁窗口
            cvDestroyWindow("dst");  
            return;  
        }  
        tmp1 = cvCreateImage(cvSize(width,height),org->depth,org->nChannels);  
        CvRect rect;  
        if(pre_pt.x<cur_pt.x && pre_pt.y<cur_pt.y)  
        {  
            rect=cvRect(pre_pt.x,pre_pt.y,width,height);  
        }  
        else if(pre_pt.x>cur_pt.x && pre_pt.y<cur_pt.y)  
        {  
            rect=cvRect(cur_pt.x,pre_pt.y,width,height);  
        }  
        else if(pre_pt.x>cur_pt.x && pre_pt.y>cur_pt.y)  
        {  
            rect=cvRect(cur_pt.x,cur_pt.y,width,height);  
        }  
        else if(pre_pt.x<cur_pt.x && pre_pt.y>cur_pt.y)  
        {  
            rect=cvRect(pre_pt.x,cur_pt.y,width,height);  
        }  
        cvSetImageROI(org,rect);//设置图像的感兴趣区域  
        cvCopy(org,tmp1); //将感兴趣区域复制到tmp1 
        cvResetImageROI(org);//释放图像的感兴趣区域  
        cvDestroyWindow("dst");//销毁上次的显示图像  
        cvNamedWindow("dst",1);//新建窗口  
        cvShowImage("dst",tmp1); //显示感兴趣的图像 
        cvSaveImage("dst.jpg",tmp1); //保存感兴趣图像 
/******************************************************************/
	}
}

int main(int argc,char *argv[])
{
    int i;
    string video_dir = "video.MOV";
    CvCapture *capture = cvCreateFileCapture(video_dir.c_str());
    while(1)
    {
    src = cvQueryFrame(capture);
//    src=cvLoadImage(argv[1]);//读入图像
//     tmp=cvCloneImage(src);//复制图像到临时图像上 
//     org=cvCloneImage(src);//保存原始图像 
    cvNamedWindow("src",CV_WINDOW_AUTOSIZE);//新建窗口
//    cvMoveWindow("src",0,0);
//     cvSetMouseCallback( "src", on_mouse, 0 );//注册鼠标响应回调函数	
/*    cvShowImage("src",src);*///显示图像
    //-----------------------------------------灰度图像-----------------------------------------//
	//图像灰度化  
    IplImage* gray_image = cvCreateImage(cvGetSize(src),8,1);  
    cvCvtColor(src,gray_image,CV_BGR2GRAY);
    //进行高斯处理，处理的是指针img指向的内存，将处理后的数据交给out指针指向的内存，对每个像素周围3x3的区域进行高斯平滑处理（其实输入输出图像可以是相同的）
    cvSmooth(gray_image,gray_image,CV_GAUSSIAN,9,9);
    //-----------------------------------------二值图像-----------------------------------------//
    //图像二值化
    IplImage*bw_image = cvCreateImage(cvGetSize(gray_image),IPL_DEPTH_8U, 1);
    double max_val = 255; //阈值化后的最大值
    cvThreshold(gray_image, bw_image, 170, max_val , CV_THRESH_BINARY); //调用OTSU算法的参数设置---CV_THRESH_OTSU
    //-----------------------------------------腐蚀图像-----------------------------------------//
    IplConvKernel *elem = cvCreateStructuringElementEx(2, 2, 0, 0,CV_SHAPE_RECT,NULL);
    IplImage *dst=cvCreateImage(cvGetSize(bw_image),IPL_DEPTH_8U, 1);
    cvErode(bw_image,dst,elem);
    //----------------------------------------------------------------------------------------//
    
    IplImage *canny_image = cvCreateImage(cvGetSize(dst), 8, 1);
    cvCanny(dst,canny_image,3,9,3);

#if 1
    /************************************************Circle identification********************************************/  
    CvMemStorage* storagecircle = cvCreateMemStorage(0);
    CvMemStorage *storage = cvCreateMemStorage(0);  
    CvSeq *first_contour = NULL;  
    
    double dp=10;
    double min_dist=1000;
    int min_radius=1;
    int max_radius=100; 
    //only greyimage is needed. cvHoughCircles would call cvSobel() internally.
    CvSeq* circles = cvHoughCircles( 
	canny_image, 
	storagecircle, 
	CV_HOUGH_GRADIENT, 
	dp, 
	min_dist, 
	min_radius, 
	max_radius );
    cvFindContours(dst, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);  
    int circlecnt=0;
    for( i = 0; i < circles->total; i++ )
    {
	float* p = (float*)cvGetSeqElem( circles, i );
	int RectCnt=0;
	for(; first_contour != 0; first_contour = first_contour->h_next)  
	{  
	  CvRect rect = cvBoundingRect(first_contour,0);
	  if(((rect.x + rect.width/2) - cvRound(p[0]))*((rect.x + rect.width/2) - cvRound(p[0])) + ((rect.y + rect.height/2) - cvRound(p[1]))*((rect.y + rect.height/2) - cvRound(p[1])) < cvRound(p[2])*cvRound(p[2]))
	  {
	    RectCnt++;
	  }	  
	}
	if(RectCnt>=4){
	  cvCircle( src, cvPoint(cvRound(p[0]),cvRound(p[1])),3, CV_RGB(255,0,0), -1, 4, 0 );
	  circlecnt++;
	}
    }
    CvFont font;  
    cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 0.5, 0.5, 1, 2, 8);  
    char textp[20];
    sprintf(textp,"target num %d",circlecnt);
    cvPutText(src, textp, cvPoint(50, 50), &font, CV_RGB(255,0,0));
    cvShowImage("src",src);
    #endif
    cvReleaseImage(&gray_image);//释放图像
    cvReleaseImage(&bw_image);//释放图像
    cvReleaseImage(&dst);//释放图像
    cvReleaseStructuringElement(&elem);
    cvReleaseImage(&canny_image);
    cvReleaseMemStorage(&storagecircle);
    cvReleaseMemStorage(&storage);
    if(cvWaitKey(1) == 27)
      break;
    }
    cvWaitKey(0);//等待按键按下 
    cvDestroyAllWindows();//销毁所有窗口
    cvReleaseImage(&src);//释放图像
    cvReleaseImage(&tmp);//释放图像
    cvReleaseImage(&org);//释放图像
    return 0;
}
