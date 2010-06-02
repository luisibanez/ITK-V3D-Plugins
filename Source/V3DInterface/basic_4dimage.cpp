/*
 * basic_4dimage.cpp
 *
 *  Extract from v3d/my4dimage.cpp
 *  Aug 17, 2009: Zongcai Ruan
 *
 */

#include "v3d_message.h"

#include "stackutil.h"
#include "basic_4dimage.h"

typedef unsigned short int USHORTINT16;



void Image4DSimple::loadImage(char filename[])
{
	cleanExistData(); /* note that this variable must be initialized as NULL. */

	strcpy(imgSrcFile, filename);

	V3DLONG * tmp_sz = 0; /* note that this variable must be initialized as NULL. */
	int tmp_datatype = 0;

	//060815, 060924, 070805
	char * curFileSurfix = getSurfix(imgSrcFile);
	printf("The current input file has the surfix [%s]\n", curFileSurfix);
	if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0) //read tiff stacks
	{
		if (loadTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			v3d_msg("Error happens in TIF file reading. Stop. \n", false);
			b_error=1;
			return;
		}
	}
	else if ( strcasecmp(curFileSurfix, "lsm")==0 ) //read lsm stacks
	{
		if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			v3d_msg("Error happens in LSM file reading. Stop. \n", false);
			b_error=1;
			return;
		}
	}
	else if ( strcasecmp(curFileSurfix, "mrc")==0 ) //read mrc stacks
	{
		if (loadMRC2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			v3d_msg("Error happens in MRC file reading. Stop. \n", false);
			b_error=1;
			return;
		}
	}
	else //then assume it is Hanchuan's RAW format
	{
		v3d_msg("The data is not with a TIF surfix, -- now this program assumes it is RAW format defined by Hanchuan Peng. \n", false);
		if (loadRaw2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			printf("The data doesn't look like a correct 4-byte-size RAW file. Try 2-byte-raw. \n");
			if (loadRaw2Stack_2byte(imgSrcFile, data1d, tmp_sz, tmp_datatype))
			{
				v3d_msg("Error happens in reading 4-byte-size and 2-byte-size RAW file. Stop. \n", false);
				b_error=1;
				return;
			}
		}
	}

	//080302: now convert any 16 bit or float data to the range of 0-255 (i.e. 8bit)
	switch (tmp_datatype)
	{
		case 1:
			datatype = V3D_UINT8;
			break;

		case 2: //080824
			//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
			//datatype = UINT8; //UINT16;
			datatype = V3D_UINT16;
			break;

		case 4:
			convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
			datatype = V3D_UINT8; //FLOAT32;
			break;

		default:
			v3d_msg("The data type is not UINT8, UINT16 or FLOAT32. Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
			if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}
			return;
	}

	sz0 = tmp_sz[0];
	sz1 = tmp_sz[1];
	sz2 = tmp_sz[2];
	sz3 = tmp_sz[3]; //no longer merge the 3rd and 4th dimensions

	/* clean all workspace variables */

	if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}

	return;
}

bool convert_data_to_8bit(void * &img, V3DLONG * sz, int datatype)
{
	if (!img || !sz)
	{
		fprintf(stderr, "The input to convert_data_to_8bit() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (datatype!=2 && datatype!=4)
	{
		fprintf(stderr, "This function convert_type2uint8_3dimg_1dpt() is designed to convert 16 bit and single-precision-float only [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1)
	{
		fprintf(stderr, "Input image size is not valid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	V3DLONG totalunits = sz[0] * sz[1] * sz[2] * sz[3];
	unsigned char * outimg = new unsigned char [totalunits];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (datatype==2) //following is new method 090718, PHC
	{
		USHORTINT16 * tmpimg = (USHORTINT16 *)img;
		V3DLONG i; double maxvv=tmpimg[0];
		for (i=0;i<totalunits;i++)
		{
			maxvv = (maxvv<tmpimg[i]) ? tmpimg[i] : maxvv;
		}
		if (maxvv>255.0)
		{
			maxvv = 255.0/maxvv;
			for (V3DLONG i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(double(tmpimg[i])*maxvv);
			}
		}
		else
		{
			for (V3DLONG i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(tmpimg[i]); //then no need to rescale
			}
		}
	}
	else
	{
		float * tmpimg = (float *)img;
		V3DLONG i; double maxvv=tmpimg[0], minvv=tmpimg[0];
		for (i=0;i<totalunits;i++)
		{
			if (tmpimg[i]>maxvv) maxvv = tmpimg[i];
			else if (tmpimg[i]<minvv) minvv = tmpimg[i];
		}
		if (maxvv!=minvv)
		{
			double w = 255.0/(maxvv-minvv);
			for (V3DLONG i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(double(tmpimg[i]-minvv)*w);
			}
		}
		else
		{
			for (V3DLONG i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(tmpimg[i]); //then no need to rescale. If the original value is small than 0 or bigger than 255, then let it be during the type-conversion
			}
		}
	}

	//copy to output data

	delete [] ((unsigned char *)img); //as I know img was originally allocated as (unsigned char *)
	img = outimg;

	return true;
}

bool Image4DSimple::createImage(V3DLONG mysz0, V3DLONG mysz1, V3DLONG mysz2, V3DLONG mysz3, ImagePixelType mytype)
{
    if (mysz0<=0 || mysz1<=0 || mysz2<=0 || mysz3<=0) return false; //note that for this sentence I don't change b_error flag
	if (data1d) {delete []data1d; data1d=0; sz0=0; sz1=0; sz2=0;sz3=0; datatype=V3D_UNKNOWN;}
	try //081001
	{
		switch (mytype)
		{
			case V3D_UINT8:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3];
				if (!data1d) {b_error=1;return false;}
				break;
			case V3D_UINT16:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3*2];
				if (!data1d) {b_error=1;return false;}
				break;
			case V3D_FLOAT32:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3*4];
				if (!data1d) {b_error=1;return false;}
				break;
			default:
				b_error=1;
				return false;
				break;
		}
		sz0=mysz0; sz1=mysz1; sz2=mysz2;sz3=mysz3; datatype=mytype; b_error=0; //note that here I update b_error
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in My4DImageSimple::createImage();\n");
		data1d = 0;
		b_error=1;
		return false;
	}
	return true;
}


void Image4DSimple::createBlankImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype)
{
	if (imgsz0<=0 || imgsz1<=0 || imgsz2<=0 || imgsz3<=0)
	{
		v3d_msg("Invalid size parameters in createBlankImage(). Do nothing. \n");
		return;
	}
	if (imgdatatype!=1 && imgdatatype!=2 && imgdatatype!=4)
	{
		v3d_msg("Invalid image datatype parameter in createBlankImage(). Do nothing. \n");
		return;
	}

	//otherwise good to go

	cleanExistData(); /* note that this variable must be initialized as NULL. */

	strcpy(imgSrcFile, "Untitled_blank.raw");

	//==============

	switch (imgdatatype)
	{
		case 1:
			datatype = V3D_UINT8;
			break;

		case 2:
			datatype = V3D_UINT16;
			break;

		case 4:
			datatype = V3D_FLOAT32;
			break;

		default:
			v3d_msg("Something wrong with the program in My4DImageSimple::createBlankImage(), -- should NOT display this message at all. Check your program. \n");
			b_error=1;
			return;
	}

	sz0 = imgsz0;
	sz1 = imgsz1;
	sz2 = imgsz2;
	sz3 = imgsz3;

	V3DLONG totalbytes = sz0*sz1*sz2*sz3*imgdatatype;
	try { //081001
		data1d = new unsigned char [totalbytes];
	}catch (...) {data1d=0;}

	if (!data1d)
	{
		v3d_msg("Fail to allocate memory in My4DImageSimple::createBlankImage(). Check your program. \n");
		sz0=sz1=sz2=sz3=0; datatype=V3D_UNKNOWN;
		b_error=1;
		return;
	}

	for (V3DLONG i=0;i<totalbytes;i++) data1d[i] = 0;

	return;
}

bool Image4DSimple::saveImage(const char filename[])
{
	if (!data1d || !filename)
	{
		v3d_msg("This image data is empty or the file name is invalid. Nothing done.\n");
		return false;
	}

	V3DLONG mysz[4];
	mysz[0] = sz0;
	mysz[1] = sz1;
	mysz[2] = sz2;
	mysz[3] = sz3;

	int dt;
	switch (datatype)
	{
		case V3D_UINT8:  dt=1; break;
		case V3D_UINT16:  dt=2; break;
		case V3D_FLOAT32:  dt=4; break;
		default:
			v3d_msg("The data type is unsupported. Nothing done.\n");
			return false;
			break;
	}

	//061009
	char * curFileSurfix = getSurfix((char *)filename);
	printf("The current output file has the surfix [%s]\n", curFileSurfix);
	if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0) //read tiff stacks
	{
		if (saveStack2Tif(filename, data1d, mysz, dt)!=0)
		{
			v3d_msg("Error happens in TIF file writing. Stop. \n");
			b_error=1;
			return false;
		}
	}
	else //then assume it is Hanchuan's RAW format
	{
		printf("The data is not with a TIF surfix, -- now this program assumes it is RAW format defined by Hanchuan Peng. \n");
		if (saveStack2Raw(filename, data1d, mysz, dt)!=0)   //0 is no error //note that as I updated the saveStack2Raw to RAW-4-byte, the actual mask file cannot be read by the old wano program, i.e. the wano must be updated on Windows machine as well. 060921
			//if (saveStack2Raw_2byte(filename, data1d, mysz, dt)!=0)   //for compatability save it to 2-byte raw //re-commented on 081124. always save to 4-byte raw
		{
			v3d_msg("Fail to save data to file [%s].\n", filename);
			b_error=1;
			return false;
		}
	}

	return true;
}


