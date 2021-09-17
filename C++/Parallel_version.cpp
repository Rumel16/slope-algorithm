
#include <iostream>
#include <gdal_priv.h>
#include <openacc.h>
#include <cpl_conv.h>
#include <cmath>
#include <ctime>

using namespace std;

int main()
{
	int unsigned nCols, nRows; 
	int noData;
	const char *pszFile;	 
	int r, c;				  
	float p, q;				  
	clock_t begin, end;		 

	int z1, z2, z3, z4, z5, z6, z7, z8;

	GDALAllRegister();
	CPLPushErrorHandler(CPLQuietErrorHandler);

	pszFile = "./40x40.tif";

	//ODCZYT

	GDALDataset *dem = (GDALDataset *)GDALOpen(pszFile, GA_ReadOnly);
	double geoTransform[6];
	dem->GetGeoTransform(geoTransform);
	GDALRasterBand *rasterBand = dem->GetRasterBand(1);
	nCols = rasterBand->GetXSize();
	nRows = rasterBand->GetYSize();
	noData = rasterBand->GetNoDataValue();

	cout << "nCols: " << nCols << " nRows: " << nRows << "\n";

	//ZAPIS

	GDALDataset *geotiffDataset;
	GDALDriver *driverGeotiff;
	driverGeotiff = GetGDALDriverManager()->GetDriverByName("GTiff");
	geotiffDataset = driverGeotiff->Create("slope.tif", nCols, nRows, 1, GDT_Float32, NULL);
	geotiffDataset->SetGeoTransform(geoTransform);
	geotiffDataset->SetProjection(dem->GetProjectionRef());

	float * restrict tab = (float *)CPLMalloc(sizeof(float) * (nCols * nRows));
	float * restrict slope = (float *)CPLMalloc(sizeof(float) * ((nCols) * (nRows)));

	unsigned long long n = (nCols * nRows);
	cout << "Rozmiar tablicy: " << n << endl; 

	int odczyt, zapis; 

	odczyt = rasterBand->RasterIO(GF_Read, 0, 0, nCols, nRows, tab, nCols, nRows, GDT_Float32, 0, 0);

	begin = clock();
#pragma acc data copy(tab [0:n], slope [0:n])
	{
#pragma acc parallel
		{
#pragma acc loop collapse(2) 
			for (r = 1; r < nRows - 1; r++)
			{
				for (c = 1; c < nCols - 1; c++)
				{
					z1 = (r - 1) * nCols + (c + 1);
					z2 = (r * nCols) + (c + 1);
					z3 = (r + 1) * nCols + (c + 1);
					z4 = (r - 1) * nCols + (c - 1);
					z5 = r * nCols + (c - 1);
					z6 = (r + 1) * nCols + (c - 1);
					z7 = (r + 1) * nCols + c;
					z8 = (r - 1) * nCols + c;

					p = ((tab[z1] + 2 * tab[z2] + tab[z3]) -
						 (tab[z4] + 2 * tab[z5] + tab[z6])) /
						8;

					q = ((tab[z6] + 2 * tab[z7] + tab[z3]) -
						 (tab[z4] + 2 * tab[z8] + tab[z1])) /
						8;

					slope[r * nCols + c] = atan(sqrt((p * p) + (q * q)));
				}
			}
		}
	}
	end = clock();
	double elapsed = double(end - begin) / CLOCKS_PER_SEC;

	cout << "Czas wykonywania: " << elapsed << " [s]";
	zapis = geotiffDataset->GetRasterBand(1)->RasterIO(GF_Write, 1, 1, nCols - 1, nRows - 1, slope, nCols - 1, nRows - 1, GDT_Float32, 0, 0);

	cout << "\n"
		 << "return zapisu: " << zapis;

	CPLFree(tab);
	CPLFree(slope);
	GDALClose(dem);
	GDALClose(geotiffDataset);
	GDALDestroyDriverManager();
	return 0;
}
