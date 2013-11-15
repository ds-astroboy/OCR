// MorphologyDlg.cpp : implementation file
//

#include "Morphology.h"

//��̬ѧ�ṹԪ�صĸ���
IplConvKernel* lhStructuringElementCopy(IplConvKernel* se)
{

	IplConvKernel* copy = cvCreateStructuringElementEx( se->nCols, se->nRows, 
			se->anchorX, se->anchorY, 0, NULL );

	copy->nShiftR = se->nShiftR;

	memcpy( copy->values, se->values, sizeof(int) * se->nRows * se->nCols );

	return copy;
}

//��̬ѧ�ṹԪ�ص�ȡ��
IplConvKernel* lhStructuringElementNot(IplConvKernel* se)
{

	IplConvKernel* temp = cvCreateStructuringElementEx( se->nCols, se->nRows, 
			se->anchorX, se->anchorY, 0, NULL );

	temp->nShiftR = se->nShiftR;

	memcpy( temp->values, se->values, sizeof(int) * se->nRows * se->nCols );

	for(int i=0; i<temp->nRows * temp->nCols ; i++)
	{
		temp->values[i] = !temp->values[i] ;
	}

	return temp;
}
//��̬ѧ�ṹԪ�صĻ���
int lhStructuringElementCard(IplConvKernel* se)
{
	assert(se != NULL);
	int i, cnt = 0;

	for (i=0; i<se->nCols*se->nRows; i++)
	{
		cnt += se->values[i];
	}
	return cnt;

}

//��̬ѧ�ṹԪ�ص�ӳ��
IplConvKernel* lhStructuringElementMap(IplConvKernel* se)
{
	CvMat *mat = cvCreateMat( se->nRows,  se->nCols, CV_32SC1);
	memcpy(mat->data.i, se->values, sizeof(int) * se->nRows * se->nCols );
	cvFlip(mat, NULL, -1);

	IplConvKernel* semap = cvCreateStructuringElementEx( se->nCols, se->nRows, 
			se->nCols-se->anchorX-1, se->nRows-se->anchorY-1, 0, NULL );

	semap->nShiftR = se->nShiftR;

	memcpy( semap->values, mat->data.i, sizeof(int) * se->nRows * se->nCols );

	cvReleaseMat(&mat);

	return semap;
}

//��̬ѧ���ԽṹԪ�صĴ�������������̬ѧ�������
IplConvKernel* lhStructuringElementLine(unsigned int angle, unsigned int len)
{
	assert(len > 2);
	angle = angle%180;

	CvPoint p1 = {0};
	CvPoint p2 = {0};
	int width = cvRound(len*cos((float)angle*CV_PI/180.0));
	int height = cvRound(len*sin((float)angle*CV_PI/180.0));

	height = height >= 1 ? height : 1;

	if (width < 0)
	{
		width = width <= -1 ? width : -1;
		p1.x = 0;
		p1.y = 0;
		p2.x = -width -1;
		p2.y = height -1;
	}
	else
	{
		width = width >= 1 ? width : 1;
		p1.x = 0;
		p1.y = height -1;
		p2.x = width -1;
		p2.y = 0;
	}
	CvMat *temp = cvCreateMat(height, width, CV_32SC1);
	cvZero(temp);
	cvLine(temp, p1, p2, cvScalar(1,1,1), 1, 4, 0);

	IplConvKernel* se = cvCreateStructuringElementEx( width, height, 
			(width-1)/2, (height-1)/2,  CV_SHAPE_CUSTOM, temp->data.i );

	cvReleaseMat(&temp);
	return se;

}


//��̬ѧ������
void lhMorpOpen(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{

    cvErode( src, dst, se, iterations );

	IplConvKernel* semap = lhStructuringElementMap(se);

    cvDilate( dst, dst, semap, iterations );

	cvReleaseStructuringElement(&semap);

}

//��̬ѧ������
void lhMorpClose(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{

    cvDilate( src, dst, se, iterations );

	IplConvKernel* semap = lhStructuringElementMap(se);

    cvErode( dst, dst, semap, iterations );

	cvReleaseStructuringElement(&semap);

}

//��̬ѧ�����ݶ�����
void lhMorpGradient(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{
	assert(src != NULL && dst != NULL && src != dst);
	IplImage*  temp = cvCloneImage(src);
	cvErode( src, temp, se, iterations );
    cvDilate( src, dst, se, iterations );
    cvSub( dst, temp, dst );
	cvReleaseImage(&temp);
}

//��̬ѧ���ݶ�����
void lhMorpGradientIn(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{
	assert(src != NULL && dst != NULL && src != dst);
	cvErode( src, dst, se, iterations );
    cvSub( src, dst, dst );
}

//��̬ѧ���ݶ�����
void lhMorpGradientOut(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{
	assert(src != NULL && dst != NULL && src != dst);
	cvDilate( src, dst, se, iterations );
    cvSub(dst, src, dst );
}

//��̬ѧ�����ݶ�
void lhMorpGradientDir(const IplImage* src, IplImage* dst, unsigned int angle, unsigned int len )
{
	assert(src != NULL && dst != NULL && src != dst);
	IplConvKernel* se = lhStructuringElementLine(angle, len);
	lhMorpGradient(src, dst, se);
	cvReleaseStructuringElement(&se);
}

//��̬ѧ�׶�ñ����
void lhMorpWhiteTopHat(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{
	assert(src != NULL && dst != NULL && src != dst);
	lhMorpOpen(src, dst, se, iterations );
    cvSub( src, dst, dst );
}


//��̬ѧ�ڶ�ñ����
void lhMorpBlackTopHat(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{
	assert(src != NULL && dst != NULL && src != dst);
	lhMorpClose(src, dst, se, iterations );
    cvSub(dst, src, dst );

}

//��̬ѧ�Բ���ñ����
void lhMorpQTopHat(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{
	assert(src != NULL && dst != NULL && src != dst);
	IplImage*  temp = cvCloneImage(src);
    lhMorpClose( src, temp, se, iterations );
    lhMorpOpen( src, dst, se, iterations );
    cvSub(temp, dst, dst );
	cvReleaseImage(&temp);
}

//��̬ѧ�Աȶ���ǿ����
void lhMorpEnhance(const IplImage* src, IplImage* dst, IplConvKernel* se=NULL, int iterations=1)
{
	assert(src != NULL && dst != NULL && src != dst);
	IplImage*  temp = cvCloneImage(src);
    lhMorpWhiteTopHat( src, temp, se, iterations );
    lhMorpBlackTopHat( src, dst, se, iterations );
	cvAdd(src, temp, temp);
    cvSub(temp, dst, dst );
	cvReleaseImage(&temp);
}

//��̬ѧ��ֵ����-�����б任
void lhMorpHMTB(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL)
{
	assert(src != NULL && dst != NULL && src != dst && sefg!= NULL && sefg!=sebg);

	if (sebg == NULL)
	{
		sebg = lhStructuringElementNot(sefg);

	}
	IplImage*  temp1 = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  temp2 = cvCreateImage(cvGetSize(src), 8, 1);

	//P104 (5.2)
	cvErode( src, temp1, sefg);
	cvNot(src, temp2);
	cvErode( temp2, temp2, sebg);
	cvAnd(temp1, temp2, dst);


	cvReleaseImage(&temp1);
	cvReleaseImage(&temp2);

	cvReleaseStructuringElement(&sebg);

}


//��̬ѧ��Լ������-�����б任 ��Զ�ֵ�ͻҶ�ͼ��
void lhMorpHMTU(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL)
{
	assert(src != NULL && dst != NULL && src != dst && sefg!= NULL && sefg!=sebg);

	if (sebg == NULL)
	{
		sebg = lhStructuringElementNot(sefg);

	}
	
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  mask = cvCreateImage(cvGetSize(src), 8, 1);
	cvZero(mask);

	//P106 (5.4)
	cvErode( src, temp, sefg);
	cvDilate(src, dst, sebg);
	cvCmp(temp, dst, mask, CV_CMP_GT);

	cvSub(temp, dst, dst, mask);
	cvNot(mask, mask);
	cvSet(dst, cvScalar(0), mask);

	//cvCopy(dst, mask);
	//cvSet(dst, cvScalar(255), mask);
	cvReleaseImage(&mask);
	cvReleaseImage(&temp);

	cvReleaseStructuringElement(&sebg);
}

//��̬ѧԼ������-�����б任 ��Զ�ֵ�ͻҶ�ͼ��
void lhMorpHMTC(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL)
{
	assert(src != NULL && dst != NULL && src != dst && sefg!= NULL && sefg!=sebg);

	if (sebg == NULL)
	{
		sebg = lhStructuringElementNot(sefg);

	}
	
	IplImage*  temp1 = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  temp2 = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  temp3 = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  temp4 = cvCreateImage(cvGetSize(src), 8, 1);

	IplImage*  mask1 = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  mask2 = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  mask3 = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  mask4 = cvCreateImage(cvGetSize(src), 8, 1);

	cvZero(mask1);
	cvZero(mask2);
	cvZero(mask3);
	cvZero(mask4);

	cvZero(dst);

	//P107 (5.5)
	cvErode( src, temp1, sebg);
	cvDilate( src, temp2, sebg);
	cvErode( src, temp3, sefg);
	cvDilate( src, temp4, sefg);

	cvCmp(src, temp3, mask1, CV_CMP_EQ);
	cvCmp(temp2, src,  mask2, CV_CMP_LT);
	cvAnd(mask1, mask2, mask2);

	cvCmp(src, temp4, mask3 , CV_CMP_EQ);
	cvCmp(temp1, src, mask4 , CV_CMP_GT);
	cvAnd(mask3, mask4, mask4);

	cvSub(src, temp2, dst, mask2);
	cvSub(temp1, src, dst, mask4);




	cvReleaseImage(&mask1);
	cvReleaseImage(&mask2);
	cvReleaseImage(&mask3);
	cvReleaseImage(&mask4);

	cvReleaseImage(&temp1);
	cvReleaseImage(&temp2);
	cvReleaseImage(&temp3);
	cvReleaseImage(&temp4);

	cvReleaseStructuringElement(&sebg);

}

#define LH_MORP_TYPE_BINARY			0
#define LH_MORP_TYPE_UNCONSTRAIN	1
#define LH_MORP_TYPE_CONSTRAIN		2

//��̬ѧԼ������-�����б任
void lhMorpHMT(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL, int type=LH_MORP_TYPE_BINARY)
{
	switch(type)
	{
	case LH_MORP_TYPE_BINARY:
		{
			lhMorpHMTB(src, dst, sefg, sebg);
			break;
		}

	case LH_MORP_TYPE_UNCONSTRAIN:
		{
			lhMorpHMTU(src, dst, sefg, sebg);
			break;
		}

	case LH_MORP_TYPE_CONSTRAIN:
		{
			lhMorpHMTC(src, dst, sefg, sebg);
			break;
		}
		
	default:
		break;

	}

}

//��̬ѧ����-�����п��任 
void lhMorpHMTOpen(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL, int type=LH_MORP_TYPE_BINARY)
{
	assert(src != NULL && dst != NULL && src != dst && sefg!= NULL && sefg!=sebg);

	IplConvKernel* semap = lhStructuringElementMap(sefg);

	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);

	//P110 (5.8)
	lhMorpHMT(src, temp, sefg, sebg, type);
	cvDilate(temp, dst, semap);

	cvReleaseImage(&temp);
	cvReleaseStructuringElement(&semap);

}

//��̬ѧϸ������
void lhMorpThin(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL, int type=LH_MORP_TYPE_BINARY)
{

	assert(src != NULL && dst != NULL && src != dst && sefg!= NULL && sefg!=sebg);


	lhMorpHMT(src, dst, sefg, NULL, type);
	cvSub(src, dst, dst);

}

//��̬ѧϸ��ƥ������
void lhMorpThinFit(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL, int type=LH_MORP_TYPE_BINARY)
{

	assert(src != NULL && dst != NULL && src != dst && sefg!= NULL && sefg!=sebg);

	lhMorpHMTOpen(src, dst, sefg, NULL, type);
	cvSub(src, dst, dst);
}

//��̬ѧ�ֻ�����
void lhMorpThick(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL, int type=LH_MORP_TYPE_BINARY)
{

	assert(src != NULL && dst != NULL && src != dst && sefg!= NULL && sefg!=sebg);


	lhMorpHMT(src, dst, sefg, NULL, type);
	cvAdd(src, dst, dst);

}

//��̬ѧ�ֻ���ƥ������
void lhMorpThickMiss(const IplImage* src, IplImage* dst, IplConvKernel* sefg, IplConvKernel* sebg =NULL, int type=LH_MORP_TYPE_BINARY)
{

	assert(src != NULL && dst != NULL && src != dst && sefg!= NULL && sefg!=sebg);

	lhMorpHMTOpen(src, dst, sefg, NULL, type);
	cvAdd(src, dst, dst);
}


//�Ƚ�����ͼ���Ƿ���ͬ�� 0 ��ͬ
int  lhImageCmp(const IplImage* img1, const IplImage* img2)
{
	assert(img1->width == img2->width && img1->height == img2->height && img1->imageSize == img2->imageSize );
	return memcmp(img1->imageData, img2->imageData, img1->imageSize);
}

//��̬ѧ������ͺ������ؽ�����
void lhMorpRDilate(const IplImage* src, const IplImage* msk, IplImage* dst, IplConvKernel* se = NULL, int iterations=-1)
{

	assert(src != NULL && msk != NULL && dst != NULL && src != dst );

	if(iterations < 0)
	{
		//�����ؽ�
		cvMin(src, msk, dst);
		cvDilate(dst, dst, se);
		cvMin(dst, msk, dst);

		IplImage*  temp1 = cvCreateImage(cvGetSize(src), 8, 1);
		//IplImage*  temp2 = cvCreateImage(cvGetSize(src), 8, 1);

		do
		{
			//record last result
			cvCopy(dst, temp1);
			cvDilate(dst, dst, se);
			cvMin(dst, msk, dst);
			//cvCmp(temp1, dst, temp2, CV_CMP_NE );

		}
		//while(cvSum(temp2).val[0] != 0);
		while(lhImageCmp(temp1, dst)!= 0);

		cvReleaseImage(&temp1);
		//cvReleaseImage(&temp2);

		return;	

	}
	else if (iterations == 0)
	{
		cvCopy(src, dst);
	}
	else
	{

		//��ͨ������� p136(6.1)
		cvMin(src, msk, dst);
		cvDilate(dst, dst, se);
		cvMin(dst, msk, dst);

		for(int i=1; i<iterations; i++)
		{
			cvDilate(dst, dst, se);
			cvMin(dst, msk, dst);

		}

	}
}

//��̬ѧ��ظ�ʴ�͸�ʴ�ؽ�����
void lhMorpRErode(const IplImage* src,  const IplImage* msk, IplImage* dst, IplConvKernel* se = NULL, int iterations=-1)
{

	assert(src != NULL  && msk != NULL && dst != NULL && src != dst );

	if(iterations < 0)
	{
		//��ʴ�ؽ�
		cvMax(src, msk, dst);
		cvErode(dst, dst, se);
		cvMax(dst, msk, dst);

		IplImage*  temp1 = cvCreateImage(cvGetSize(src), 8, 1);
		//IplImage*  temp2 = cvCreateImage(cvGetSize(src), 8, 1);

		do
		{
			//record last result
			cvCopy(dst, temp1);
			cvErode(dst, dst, se);
			cvMax(dst, msk, dst);
			//cvCmp(temp1, dst, temp2, CV_CMP_NE);

		}
		//while(cvSum(temp2).val[0] != 0);
		while(lhImageCmp(temp1, dst)!= 0);

		cvReleaseImage(&temp1);
		//cvReleaseImage(&temp2);

		return;	

	}
	else if (iterations == 0)
	{
		cvCopy(src, dst);
	}
	else
	{
		//��ͨ��ظ�ʴ p137(6.2)
		cvMax(src, msk, dst);
		cvErode(dst, dst, se);
		cvMax(dst, msk, dst);

		for(int i=1; i<iterations; i++)
		{
			cvErode(dst, dst, se);
			cvMax(dst, msk, dst);
		}
	}
}

//��̬ѧ���ؽ�
void lhMorpROpen(const IplImage* src, IplImage* dst, IplConvKernel* se = NULL, int iterations=1)
{
	assert(src != NULL  && dst != NULL && src != dst );

	//p155(6.16)
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);
	cvErode(src, temp, se, iterations);
	lhMorpRDilate(temp, src, dst, se, -1);
	cvReleaseImage(&temp);

}

//��̬ѧ���ؽ�
void lhMorpRClose(const IplImage* src, IplImage* dst, IplConvKernel* se = NULL, int iterations=1)
{
	assert(src != NULL  && dst != NULL && src != dst );

	//p155(6.17)
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);
	cvDilate(src, temp, se, iterations);
	lhMorpRErode(temp, src, dst, se, -1);
	cvReleaseImage(&temp);

}

//��̬ѧ��ñ�ؽ�
void lhMorpRWTH(const IplImage* src, IplImage* dst, IplConvKernel* se = NULL, int iterations=1)
{
	assert(src != NULL  && dst != NULL && src != dst );
	//p156
	lhMorpROpen(src, dst, se, iterations);
	cvSub(src, dst, dst);
}

//��̬ѧ��ñ�ؽ�
void lhMorpRBTH(const IplImage* src, IplImage* dst, IplConvKernel* se = NULL, int iterations=1)
{
	assert(src != NULL  && dst != NULL && src != dst );
	//p156
	lhMorpRClose(src, dst, se, iterations);
	cvSub(dst, src, dst);
}


//��̬ѧ����Զ�ż���Զ�ż�ؽ�����
void lhMorpRSelfDual(const IplImage* src, const IplImage* msk, IplImage* dst, IplConvKernel* se = NULL, int iterations=-1)
{
	assert(src != NULL  && msk != NULL && dst != NULL && src != dst );

	//p140(6.7) p142 (6.10)
	IplImage*  temp1 = cvCreateImage(cvGetSize(src), 8, 1);
	IplImage*  temp2 = cvCreateImage(cvGetSize(src), 8, 1);

	cvZero(temp2);

	lhMorpRDilate(src, msk, temp1, se, iterations);

	lhMorpRErode(src, msk, dst, se, iterations);

	cvCmp(src, msk, temp2, CV_CMP_LE);

	cvCopy(temp1, dst, temp2);

	cvReleaseImage(&temp1);
	cvReleaseImage(&temp2);
}



//��̬ѧ����Сֵ
void lhMorpRMin(const IplImage* src, IplImage* dst, IplConvKernel* se = NULL)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	//p149 (6.14)
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);

	cvAddS(src, cvScalar(1), temp);
	
	lhMorpRErode(temp, src, dst, se);

	cvSub(dst, src, dst);

	cvReleaseImage(&temp);

}

//��̬ѧ���򼫴�ֵ
void lhMorpRMax(const IplImage* src, IplImage* dst, IplConvKernel* se = NULL)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	//p149 (6.13)
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);

	cvSubS(src, cvScalar(1), temp);
	
	lhMorpRDilate(temp, src, dst, se);

	cvSub(src, dst, dst);

	cvReleaseImage(&temp);

}

//��̬ѧH����ֵ
void lhMorpHMax(const IplImage* src, IplImage* dst, unsigned char h, IplConvKernel* se = NULL)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	//p150
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);

	cvSubS(src, cvScalar(h), temp);
	
	lhMorpRDilate(temp, src, dst, se);

	cvReleaseImage(&temp);

}


//��̬ѧH��Сֵ
void lhMorpHMin(const IplImage* src, IplImage* dst, unsigned char h, IplConvKernel* se = NULL)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	//p150
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);

	cvAddS(src, cvScalar(h), temp);
	
	lhMorpRErode(temp, src, dst, se);

	cvReleaseImage(&temp);

}

//��̬ѧH���任
void lhMorpHConcave(const IplImage* src, IplImage* dst, unsigned char h, IplConvKernel* se = NULL)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	//p150

	lhMorpHMin(src, dst, h, se);
	cvSub(dst, src, dst);

}

//��̬ѧH͹�任
void lhMorpHConvex(const IplImage* src, IplImage* dst, unsigned char h, IplConvKernel* se = NULL)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	//p150

	lhMorpHMax(src, dst, h, se);
	cvSub(src, dst, dst);

}

//��̬ѧ��չ����ֵ
void lhMorpEMax(const IplImage* src, IplImage* dst, unsigned char h, IplConvKernel* se = NULL)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	//p150
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);

	lhMorpHMax(src, temp, h, se);
	lhMorpRMax(temp, dst, se);

	cvReleaseImage(&temp);

}

//��̬ѧ��չ��Сֵ
void lhMorpEMin(const IplImage* src, IplImage* dst, unsigned char h, IplConvKernel* se = NULL)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	//p150
	IplImage*  temp = cvCreateImage(cvGetSize(src), 8, 1);

	lhMorpHMin(src, temp, h, se);
	lhMorpRMin(temp, dst, se);

	cvReleaseImage(&temp);

}


//��̬ѧ�ȼ��˲�������ֵ,Ĭ��SEΪ����3*3��
void lhMorpRankFilterB(const IplImage* src, IplImage* dst, IplConvKernel* se = NULL, unsigned int rank = 0)
{
	assert(src != NULL  &&  dst != NULL && src != dst );

	bool defaultse = false;
	int card;
	if (se == NULL)
	{
		card = 3*3;
		assert(rank >= 0 && rank <= card);
		se = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT);
		defaultse = true;
	}
	else
	{
		card = lhStructuringElementCard(se);
		assert(rank >= 0 && rank <= card);
	}

	//default rank is median
	if (rank == 0)
		rank = card/2+1;

	IplConvKernel* semap =	lhStructuringElementMap(se);

	CvMat *semat = cvCreateMat(semap->nRows, semap->nCols, CV_32FC1);

	int i;
	for (i=0; i<semap->nRows*semap->nCols; i++)
	{
		semat->data.fl[i] = semap->values[i];
	}

	cvThreshold(src, dst, 0, 1, CV_THRESH_BINARY);
	IplImage *temp = cvCreateImage(cvGetSize(dst), 8, 1);

	cvFilter2D(dst, temp, semat, cvPoint(semap->anchorX, semap->anchorY));

	cvThreshold(temp, dst, card-rank, 255, CV_THRESH_BINARY);

	cvReleaseMat(&semat);
	cvReleaseStructuringElement(&semap);

	if (defaultse)
		cvReleaseStructuringElement(&se);	
	
	cvReleaseImage(&temp);

}

//��̬ѧ�ؽ�Ӧ��1��ȥ���߽����ͨ����
void lhMorpRemoveBoderObj(const IplImage* src, IplImage* dst)
{
	IplImage *temp = cvCloneImage(src);
	//double min, max;
	//cvMinMaxLoc(src, &min, &max);
	
	//���ͼ��
	cvRectangle(temp, cvPoint(3,3), cvPoint(temp->width-7, temp->height-7), CV_RGB(0,0,0), -1);

	//��ԭͼ����Ϊ��ģͼ��
	lhMorpRDilate(temp, src, dst);

	cvReleaseImage(&temp);
	//cvSet((CvArr*)src, cvScalar(min), dst);
	//cvCopy(src, dst);
	cvSub(src, dst, dst);
}


//��̬ѧ�ؽ�Ӧ��2���ն������
void lhMorpFillHole(const IplImage* src, IplImage* dst)
{
	IplImage *temp = cvCloneImage(src);
	double min, max;
	cvMinMaxLoc(src, &min, &max);
	//���ͼ��
	cvRectangle(temp, cvPoint(3,3), cvPoint(temp->width-7, temp->height-7), CV_RGB(max,max,max), -1);

	//��ԭͼ����Ϊ��ģͼ��
	lhMorpRErode(temp, src, dst);

	cvReleaseImage(&temp);
	//cvSub(src, dst, dst);
}


//
//
///////////////////////////////////////////////////////////////////////////////
//// CMorphologyDlg dialog
//
//CMorphologyDlg::CMorphologyDlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CMorphologyDlg::IDD, pParent)
//{
//	//{{AFX_DATA_INIT(CMorphologyDlg)
//	m_Shape = 0;
//	m_nBinTh = 180;
//	m_nMethod = 0;
//	//}}AFX_DATA_INIT
//	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
//
//	for (int i=0; i<PLOTNUM; i++)
//	{
//		m_pOrgImage[i] = NULL;
//	}
//
//	m_pKernel = NULL;
//
//	m_nCurImgFlag = 0;
//
//
//}
//
//void CMorphologyDlg::DoDataExchange(CDataExchange* pDX)
//{
//	CDialog::DoDataExchange(pDX);
//	//{{AFX_DATA_MAP(CMorphologyDlg)
//	DDX_Control(pDX, IDC_SPIN_ROW, m_SpinRow);
//	DDX_Control(pDX, IDC_SPIN_COL, m_SpinCol);
//	DDX_Control(pDX, IDC_SPIN_ANCHORY, m_SpinAnchorY);
//	DDX_Control(pDX, IDC_SPIN_ANCHORX, m_SpinAnchorX);
//	DDX_Control(pDX, IDC_EDIT_ROW, m_RowEdit);
//	DDX_Control(pDX, IDC_EDIT_COL, m_ColEdit);
//	DDX_Control(pDX, IDC_EDIT_ANCHORY, m_AnchorYEdit);
//	DDX_Control(pDX, IDC_EDIT_ANCHORX, m_AnchorXEdit);
//	DDX_Radio(pDX, IDC_RADIO_SHAPE1, m_Shape);
//	DDX_Text(pDX, IDC_EDIT_BINARYTH, m_nBinTh);
//	DDV_MinMaxUInt(pDX, m_nBinTh, 0, 255);
//	DDX_CBIndex(pDX, IDC_COMBO_MORP, m_nMethod);
//	//}}AFX_DATA_MAP
//}
//
//BEGIN_MESSAGE_MAP(CMorphologyDlg, CDialog)
//	//{{AFX_MSG_MAP(CMorphologyDlg)
//	ON_WM_PAINT()
//	ON_WM_QUERYDRAGICON()
//	ON_BN_CLICKED(IDC_BUTTON_OPENFILE, OnButtonOpenFile)
//	ON_BN_CLICKED(IDC_BUTTON_SAVEFILE, OnButtonSaveFile)
//	ON_WM_DESTROY()
//	ON_BN_CLICKED(IDC_BUTTON_RELOAD, OnButtonReload)
//	ON_BN_CLICKED(IDC_BUTTON_KERNEL_MODIFY, OnButtonKernelModify)
//	ON_BN_CLICKED(IDC_BUTTON_BINARY, OnButtonBinary)
//	ON_BN_CLICKED(IDC_BUTTON_MORPHOLOGY, OnButtonMorphology)
//	ON_BN_CLICKED(IDC_BUTTON_UNDO, OnButtonUndo)
//	ON_BN_CLICKED(IDC_BUTTON_NOT, OnButtonNot)
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()
//
///////////////////////////////////////////////////////////////////////////////
//// CMorphologyDlg message handlers
//
//BOOL CMorphologyDlg::OnInitDialog()
//{
//	CDialog::OnInitDialog();
//
//	// Set the icon for this dialog.  The framework does this automatically
//	//  when the application's main window is not a dialog
//	SetIcon(m_hIcon, TRUE);			// Set big icon
//	SetIcon(m_hIcon, FALSE);		// Set small icon
//
//	
//	m_SpinRow.SetBuddy(&m_RowEdit);
//	m_SpinRow.SetRange(1.f, 300.0f);
//	m_SpinRow.SetPos(KERNEL_ROW);
//	m_SpinRow.SetDelta(1.0f);
//
//	m_SpinCol.SetBuddy(&m_ColEdit);
//	m_SpinCol.SetRange(1.f, 300.0f);
//	m_SpinCol.SetPos(KERNEL_COL);
//	m_SpinCol.SetDelta(1.0f);
//
//	m_SpinAnchorY.SetBuddy(&m_AnchorYEdit);
//	m_SpinAnchorY.SetRange(0.f, 300.0f);
//	m_SpinAnchorY.SetPos(ANCHOR_Y);
//	m_SpinAnchorY.SetDelta(1.0f);
//
//	m_SpinAnchorX.SetBuddy(&m_AnchorXEdit);
//	m_SpinAnchorX.SetRange(0.f, 300.0f);
//	m_SpinAnchorX.SetPos(ANCHOR_X);
//	m_SpinAnchorX.SetDelta(1.0f);
//
//
//	CStatic *pSt[PLOTNUM];
//	pSt[0] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE1);
//	pSt[1] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE2);
//	pSt[2] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE3);
//	pSt[3] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE4);	
//	
//	for (int i=0; i<PLOTNUM; i++)
//	{
//		RECT rect;
//		pSt[i]->GetWindowRect(&rect);
//		ScreenToClient(&rect);
//		m_FrameOrg[i].Create( rect.right-rect.left, rect.bottom-rect.top, 8);
//	}
//	
//
//
//	for (i=0; i<PLOTNUM; i++)
//	{
//		if (m_pOrgImage[i] == NULL)
//		{
//			m_pOrgImage[i] = cvCreateImage(cvSize(IMGWIDTH, IMGHEIGHT), 8, 1);
//			cvSetZero(m_pOrgImage[i] );
//		}
//
//	}
//	//cvLine(m_pOrgImage[3], cvPoint(100, 100), cvPoint(100, 100), CV_RGB(255, 255,255), 8);
//	//cvLine(m_pOrgImage[3], cvPoint(100, 101), cvPoint(100, 101), CV_RGB(255, 255,255));
//
//	OnButtonKernelModify();
//	
//
//	return TRUE;  // return TRUE  unless you set the focus to a control
//}
//
//// If you add a minimize button to your dialog, you will need the code below
////  to draw the icon.  For MFC applications using the document/view model,
////  this is automatically done for you by the framework.
//
//void CMorphologyDlg::OnPaint() 
//{
//	if (IsIconic())
//	{
//		CPaintDC dc(this); // device context for painting
//
//		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
//
//		// Center icon in client rectangle
//		int cxIcon = GetSystemMetrics(SM_CXICON);
//		int cyIcon = GetSystemMetrics(SM_CYICON);
//		CRect rect;
//		GetClientRect(&rect);
//		int x = (rect.Width() - cxIcon + 1) / 2;
//		int y = (rect.Height() - cyIcon + 1) / 2;
//
//		// Draw the icon
//		dc.DrawIcon(x, y, m_hIcon);
//	}
//	else
//	{
//		CPaintDC dc(this); // device context for painting
//		IplImage* ShownImage;
//		CStatic *pSt[PLOTNUM];
//		pSt[0] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE1);
//		pSt[1] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE2);
//		pSt[2] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE3);
//		pSt[3] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE4);
//
//
//		for (int i=0; i<PLOTNUM; i++)
//		{
//
//			ShownImage = m_FrameOrg[i].GetImage();
//			//ShownImage->origin = 1;
//			
//			cvResize(m_pOrgImage[i], ShownImage, CV_INTER_LINEAR );
//
//			//������Ƶ����λ��
//
//			RECT rect;
//			pSt[i]->GetWindowRect(&rect);
//			ScreenToClient(&rect);
//			m_FrameOrg[i].Show(dc.GetSafeHdc(), rect.left, rect.top, 
//				rect.right-1, rect.bottom-1, 0, 0 );
//
//		}
//
//		CDialog::OnPaint();
//	
//	}
//}
//
//// The system calls this to obtain the cursor to display while the user drags
////  the minimized window.
//HCURSOR CMorphologyDlg::OnQueryDragIcon()
//{
//	return (HCURSOR) m_hIcon;
//}
//
//void CMorphologyDlg::OnButtonOpenFile() 
//{
//	cvSetZero(m_pOrgImage[0]);
//	cvSetZero(m_pOrgImage[1]);
//
//	
//	//CFileDialog dlg(TRUE, NULL, GetCurrentPath()+"\\*.*", 
//	CFileDialog dlg(TRUE, NULL, "*.*", 
//		OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
//		"IMG files (*.bmp; *.jpg) |*.bmp;*.jpg||",NULL);
//	char title[]= {"����ͼ��"};
//	dlg.m_ofn.lpstrTitle= title;
//
//	if (dlg.DoModal() == IDOK) 
//	{
//		
//		IplImage* img = cvLoadImage(dlg.GetPathName(), 0);//CV_LOAD_IMAGE_GRAYSCALE);
//
//		m_strFilePath = dlg.GetPathName();
//		
//		SetDlgItemText(IDC_EDIT_FILEPATH, dlg.GetPathName());
//		
//		CString size;
//		size.Format("%d*%d", cvGetSize( img ).width, cvGetSize( img ).height );
//		SetDlgItemText(IDC_STATIC_IMGSIZE, size);
//
///*		if ( cvGetSize( img ).height != IMGHEIGHT  || cvGetSize( img ).width != IMGWIDTH  )
//		{
//
//			AfxMessageBox("ͼ���С��ƥ�䣺" + dlg.GetPathName());
//
//			cvReleaseImage(&img);
//
//			
//			return ;
//
//		}
//*/	
//		//cvResize(img, m_pOrgImage[0]);
//
//		ResizeImages(cvGetSize(img));
//		cvCopy(img, m_pOrgImage[0]);
//		cvCopy(img, m_pOrgImage[3]);
//
//		m_nCurImgFlag = 0;
//
//		//cvReleaseImage(&img);
//
//		//ProcessImage();
//	}
//	
//	UpdateImageRect();
//	
//}
//void CMorphologyDlg::ResizeImages(CvSize size)
//{
//	for (int i=0; i<PLOTNUM; i++)
//	{
//		if (m_pOrgImage[i] != NULL)
//		{
//			cvReleaseImage(&m_pOrgImage[i]);	
//			m_pOrgImage[i] = cvCreateImage(size, 8, 1);
//			cvSetZero(m_pOrgImage[i] );
//		}
//	}
//
//}
//
//
//void CMorphologyDlg::OnButtonSaveFile() 
//{
//	CFileDialog dlg(FALSE, ".bmp", GetCurrentPath()+"\\*.BMP", 
//		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
//		"All Files (*.bmp)|*.bmp|",NULL);
//	char title[]= {"��������ͼ��"};
//	dlg.m_ofn.lpstrTitle= title;
//
//	if (dlg.DoModal() == IDOK) 
//	{
//
//		cvSaveImage( dlg.GetPathName(), m_pOrgImage[3]);
//		SetDlgItemText(IDC_EDIT_FILEPATH2, dlg.GetPathName() );
//		
//	}
//	
//}
//
//CString CMorphologyDlg::GetCurrentPath()
//{
//
//	char buffer[MAX_PATH];
//	GetModuleFileName(NULL,buffer,MAX_PATH);
//	CString path =CString(buffer);
//	path = path.Left(path.ReverseFind('\\'));
//	return path;
//}
//
//void CMorphologyDlg::ProcessImage()
//{
//
//	int64 begin_count = cvGetTickCount();
//	
//	//cvZero(m_pOrgImage[1]);
//	//cvZero(m_pOrgImage[2]);
//	//cvZero(m_pOrgImage[3]);
//
//	cvCopyImage(m_pOrgImage[2], m_pOrgImage[1]);
//	cvCopyImage(m_pOrgImage[3], m_pOrgImage[2]);
//
//
//	IplImage *src = m_pOrgImage[2];
//	IplImage *dst = m_pOrgImage[3];
//
//
//
//
//	UpdateData(TRUE);
//	switch(m_nMethod)
//	{
//
//		//��ʴ
//		case 0:
//			{
//				cvErode(src, dst, m_pKernel);
//
//
//				break;
//			}
//
//		//����
//		case 1:
//			{
//
//				cvDilate(src, dst, m_pKernel);
//
//				break;
//			}
//
//			
//		//��
//		case 2:
//			{
//				lhMorpOpen(src, dst, m_pKernel);
//
//
//				break;
//			}
//
//		//��
//		case 3:
//			{
//
//				lhMorpClose(src, dst, m_pKernel);
//
//				break;
//			}
//
//		//��̬�ݶ�
//		case 4:
//			{
//
//				lhMorpGradient(src, dst, m_pKernel);
//
//				break;
//			}
//
//		//��̬���ݶ�
//		case 5:
//			{
//
//				lhMorpGradientIn(src, dst, m_pKernel);
//
//				break;
//			}
//
//		//��̬���ݶ�
//		case 6:
//			{
//
//				lhMorpGradientOut(src, dst, m_pKernel);
//
//				break;
//			}		
//			
//		//��ñ
//		case 7:
//			{
//
//				lhMorpWhiteTopHat(src, dst, m_pKernel);
//
//				break;
//			}
//
//		//��ñ
//		case 8:
//			{
//
//				lhMorpBlackTopHat(src, dst, m_pKernel);
//
//				break;
//			}
//
//		//�Բ���ñ
//		case 9:
//			{
//
//				lhMorpQTopHat(src, dst, m_pKernel);
//
//				break;
//			}
//
//		//�Աȶ���ǿ
//		case 10:
//			{
//
//				lhMorpEnhance(src, dst, m_pKernel);
//
//				break;
//			}
//
//		//����-������(��ֵ)
//		case 11:
//			{
//
//				lhMorpHMT(src, dst, m_pKernel, LH_MORP_TYPE_BINARY);
//				break;
//			}
//
//
//		//����-������(��Լ��)
//		case 12:
//			{
//
//				lhMorpHMT(src, dst, m_pKernel, NULL, LH_MORP_TYPE_UNCONSTRAIN);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//
//				break;
//			}
//
//
//		//����-������(Լ��)
//		case 13:
//			{
//
//				lhMorpHMT(src, dst, m_pKernel, NULL, LH_MORP_TYPE_CONSTRAIN);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//
//				break;
//			}
//
//		//����-�����п�(��ֵ)
//		case 14:
//			{
//
//				lhMorpHMTOpen(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_BINARY);
//
//				break;
//			}
//
//		//����-�����п�(��Լ��)
//		case 15:
//			{
//
//				lhMorpHMTOpen(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_UNCONSTRAIN);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//
//		//����-�����п�(Լ��)
//		case 16:
//			{
//
//				lhMorpHMTOpen(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_CONSTRAIN);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//
//		//ϸ��(��ֵ)
//		case 17:
//			{
//				lhMorpThin(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_BINARY);
//
//				break;
//			}
//
//		//ϸ��(��Լ�� �Ҷ�)
//		case 18:
//			{
//				lhMorpThin(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_UNCONSTRAIN);
//
//				break;
//			}
//
//		//ϸ��(Լ�� �Ҷ�)
//		case 19:
//			{
//				lhMorpThin(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_CONSTRAIN);
//
//				break;
//			}
//
//		//ϸ��ƥ��(��ֵ)
//		case 20:
//			{
//				lhMorpThinFit(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_BINARY);
//
//				break;
//			}
//
//		//ϸ��ƥ��(��Լ�� �Ҷ�)
//		case 21:
//			{
//				lhMorpThinFit(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_UNCONSTRAIN);
//
//				break;
//			}
//
//		//ϸ��ƥ��(Լ�� �Ҷ�)
//		case 22:
//			{
//				lhMorpThinFit(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_CONSTRAIN);
//
//				break;
//			}
//
//		//�ֻ�(��ֵ)
//		case 23:
//			{
//				lhMorpThick(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_BINARY);
//
//				break;
//			}
//
//		//�ֻ�(��Լ�� �Ҷ�)
//		case 24:
//			{
//				lhMorpThick(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_UNCONSTRAIN);
//
//				break;
//			}
//
//		//�ֻ�(Լ�� �Ҷ�)
//		case 25:
//			{
//				lhMorpThick(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_CONSTRAIN);
//
//				break;
//			}
//
//		//�ֻ���ƥ��(��ֵ)
//		case 26:
//			{
//				lhMorpThickMiss(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_BINARY);
//
//				break;
//			}
//
//		//�ֻ���ƥ��(��Լ�� �Ҷ�)
//		case 27:
//			{
//				lhMorpThickMiss(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_UNCONSTRAIN);
//
//				break;
//			}
//
//		//�ֻ���ƥ��(Լ�� �Ҷ�)
//		case 28:
//			{
//				lhMorpThickMiss(src, dst, m_pKernel,  NULL, LH_MORP_TYPE_CONSTRAIN);
//
//				break;
//			}
//
//
//		//���ؽ�
//		case 29:
//			{
//
//				lhMorpROpen(src, dst, m_pKernel, 2);
//
//				break;
//			}
//
//		//���ؽ�
//		case 30:
//			{
//				lhMorpRClose(src, dst, m_pKernel, 2);
//
//				break;
//			}
//
//		//��ñ�ؽ�
//		case 31:
//			{
//
//				//�׶�ñ�ؽ�
//				//lhMorpRWTH(src, dst, m_pKernel, 2);
//				//�ڶ�ñ�ؽ�
//				lhMorpRBTH(src, dst, m_pKernel, 2);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//
//		//����Сֵ
//		case 32:
//			{
//				lhMorpRMin(src, dst, m_pKernel);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//
//		//���򼫴�ֵ
//		case 33:
//			{
//
//				lhMorpRMax(src, dst, m_pKernel);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//
//		//H��Сֵ
//		case 34:
//			{
//				lhMorpHMin(src, dst, 100, m_pKernel);
//
//				break;
//			}
//
//		//H����ֵ
//		case 35:
//			{
//
//				lhMorpHMax(src, dst, 100, m_pKernel);
//				break;
//			}
//
//		//H���任
//		case 36:
//			{
//				lhMorpHConcave(src, dst, 100, m_pKernel);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//
//		//H͹�任
//		case 37:
//			{
//				lhMorpHConvex(src, dst, 100, m_pKernel);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//
//		//��չ��Сֵ
//		case 38:
//			{
//
//				lhMorpEMin(src, dst, 100, m_pKernel);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//
//		//��չ����ֵ
//		case 39:
//			{
//
//				lhMorpEMax(src, dst, 100, m_pKernel);
//				//Ϊ�˷�����ʾ�������ֵ��
//				cvThreshold(dst, dst, 0, 255, CV_THRESH_BINARY);
//				break;
//			}
//		//�ȼ��˲�
//		case 40:
//			{
//
//				//��ֵ�˲�
//				//lhMorpRankFilterB(src, dst, m_pKernel);
//
//				//��ʴ
//				//lhMorpRankFilterB(src, dst, m_pKernel, 1);
//
//				//����
//				lhMorpRankFilterB(src, dst, m_pKernel, lhStructuringElementCard(m_pKernel));
//				break;
//			}
//
//		//�����ݶ� ��ֱ
//		case 41:
//			{
//				//IplConvKernel* se = lhStructuringElementLine(45, 10);
//				//cvReleaseStructuringElement(&se);
//
//				lhMorpGradientDir(src, dst, 90, 3);
//
//				break;
//
//			}
//
//		//�����ݶ� ˮƽ
//		case 42:
//			{
//				lhMorpGradientDir(src, dst, 0, 3);
//				/*
//				IplConvKernel* se = lhStructuringElementLine(0, 35);
//				lhMorpOpen(src, dst, se);
//				cvReleaseStructuringElement(&se);
//
//				IplImage *temp = cvCreateImage(cvGetSize(src), 8,1 );
//				se = lhStructuringElementLine(90, 35);
//				lhMorpOpen(src, temp, se);
//				cvReleaseStructuringElement(&se);				
//
//				cvSub(src, dst, dst);
//				cvSub(dst, temp, dst);
//				cvCop
//
//				lhMorpOpen(dst, dst);
//				*/
//				break;
//			}
//
//		//�ؽ�Ӧ��1��ȥ���߽����ͨ����
//		case 43:
//			{
//
//				lhMorpRemoveBoderObj(src, dst);
//				break;
//			}
//
//		//�ؽ�Ӧ��2���ն������
//		case 44:
//			{
//
//				lhMorpFillHole(src, dst);
//				break;
//			}
//
//		default:
//			break;
//	}
//
//	UpdateImageRect();
//
//	SetDlgItemInt(IDC_STATIC_TIMECOST, (cvGetTickCount() - begin_count)/cvGetTickFrequency()/1000 );
// 
//}
//
//void CMorphologyDlg::OnDestroy() 
//{
//
//	for (int i=0; i<PLOTNUM; i++)
//	{
//		if (m_pOrgImage[i] != NULL)
//			cvReleaseImage(&m_pOrgImage[i]);	
//	}
//	if ( m_pKernel != NULL)
//	{
//		cvReleaseStructuringElement(&m_pKernel);
//		m_pKernel = NULL;
//	}
//	
//	CDialog::OnDestroy();
//	
//	
//}
//
//void CMorphologyDlg::UpdateImageRect()
//{
//	CStatic *pSt[PLOTNUM];
//	pSt[0] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE1);
//	pSt[1] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE2);
//	pSt[2] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE3);
//	pSt[3] = (CStatic *)GetDlgItem(IDC_STATIC_IMAGE4);
//
//	for (int i=0; i<PLOTNUM; i++)
//	{
//		RECT rect;
//		pSt[i]->GetWindowRect(&rect);
//		ScreenToClient(&rect);
//		InvalidateRect(&rect, FALSE);
//	}
//
//}
//
//void CMorphologyDlg::OnButtonReload() 
//{
//	if (m_strFilePath.IsEmpty())
//		return;
//
//	IplImage* img = cvLoadImage(m_strFilePath, 0);//CV_LOAD_IMAGE_GRAYSCALE);
//
//	ResizeImages(cvGetSize(img));
//	cvCopy(img, m_pOrgImage[0]);
//	cvCopy(img, m_pOrgImage[3]);
//
//	m_nCurImgFlag = 0;
//
//	UpdateImageRect();	
//}
//
//void CMorphologyDlg::OnButtonKernelModify() 
//{
//	UpdateData(TRUE);
//
//	if ( m_pKernel != NULL)
//	{
//		cvReleaseStructuringElement(&m_pKernel);
//		m_pKernel = NULL;
//	}
//
//	//��ֹanchorֵ���
//	if(m_SpinAnchorX.GetPos() >= m_SpinCol.GetPos())
//	{
//		m_SpinAnchorX.SetPos((int)m_SpinCol.GetPos()/2);
//
//	}
//
//	if(m_SpinAnchorY.GetPos() >= m_SpinRow.GetPos())
//	{
//		m_SpinAnchorY.SetPos((int)m_SpinRow.GetPos()/2);
//
//	}
//
//	if(m_Shape != 3)
//	{
//
//		m_pKernel = cvCreateStructuringElementEx( m_SpinCol.GetPos(), m_SpinRow.GetPos(), 
//			m_SpinAnchorX.GetPos(), m_SpinAnchorY.GetPos(), m_Shape );
//	}
//	else
//	{
//		//�Զ���ṹԪ��SE
//		int col = 9, row = 4;
//		int anchorx = 1, anchory = 0;
//		int kernel[] = {0, 0, 0, 1, 1, 1, 0, 0, 0 };
//		//int kernel[] = {0, 1};
//
//		m_pKernel = cvCreateStructuringElementEx( col, row, anchorx, anchory, CV_SHAPE_CUSTOM, kernel );
//
//		//���½��������ʾ
//		m_SpinCol.SetPos(col);
//		m_SpinRow.SetPos(row);
//		m_SpinAnchorX.SetPos(anchorx);
//		m_SpinAnchorY.SetPos(anchory);
//		//UpdateData(FALSE);
//	}
//
//	//int temp[256];
//	//memcpy(temp, m_pKernel->values, m_SpinCol.GetPos()*m_SpinRow.GetPos()*sizeof(int));
//
//}
//
//void CMorphologyDlg::OnButtonBinary() 
//{
//	UpdateData(TRUE);
//
//	cvCopyImage(m_pOrgImage[2], m_pOrgImage[1]);
//	cvCopyImage(m_pOrgImage[3], m_pOrgImage[2]);
//
//
//	IplImage *src = m_pOrgImage[2];
//	IplImage *dst = m_pOrgImage[3];
//	
//	cvThreshold(src, dst, m_nBinTh, 255, CV_THRESH_BINARY);
//	UpdateImageRect();
//	
//}
//
//void CMorphologyDlg::OnButtonMorphology() 
//{
//	IplImage*  temp1 = cvCreateImage(cvSize(200, 200), IPL_DEPTH_8U, 1);
//	IplImage*  temp2 = cvCreateImage(cvSize(200, 200), IPL_DEPTH_8U, 1);
//
//	cvSet(temp1, cvScalar(1));
//	cvSet(temp2, cvScalar(255));
//	cvSub(temp1, temp2, temp2);
//
//	cvReleaseImage(&temp1);
//	cvReleaseImage(&temp2);
//
//	ProcessImage();
//	
//}
//
//void CMorphologyDlg::OnButtonUndo() 
//{
//	cvCopyImage(m_pOrgImage[2], m_pOrgImage[3]);
//	cvCopyImage(m_pOrgImage[1], m_pOrgImage[2]);
//	UpdateImageRect();
//	
//}
//
//void CMorphologyDlg::OnButtonNot() 
//{
//	cvCopyImage(m_pOrgImage[2], m_pOrgImage[1]);
//	cvCopyImage(m_pOrgImage[3], m_pOrgImage[2]);
//
//
//	IplImage *src = m_pOrgImage[2];
//	IplImage *dst = m_pOrgImage[3];
//	
//	cvNot(src, dst);
//	UpdateImageRect();
//	
//}
