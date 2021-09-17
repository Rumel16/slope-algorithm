#include <iostream>
#include <gdal_priv.h>
#include <cpl_conv.h> 
#include <cmath>
#include <sys/time.h>
#include <ctime>

using namespace std;

int main()
{
	int nCols, nRows, noData;
	const char *pszFile;

	int r, c;
	float p, q; 

	clock_t begin, end;

	GDALAllRegister();
	CPLPushErrorHandler(CPLQuietErrorHandler);

	pszFile = "./file.tif"; 

	// Read
	GDALDataset *dem = (GDALDataset *)GDALOpen(pszFile, GA_ReadOnly);
	double geoTransform[6];
	dem->GetGeoTransform(geoTransform);
	GDALRasterBand *rasterBand = dem->GetRasterBand(1);
	nCols = rasterBand->GetXSize();
	nRows = rasterBand->GetYSize();
	noData = rasterBand->GetNoDataValue();

	cout << "nCols: " << nCols << " nRows: " << nRows << "\n";

	// Save
	GDALDataset *geotiffDataset;
	GDALDriver *driverGeotiff;
	driverGeotiff = GetGDALDriverManager()->GetDriverByName("GTiff");
	geotiffDataset = driverGeotiff->Create("slope.tif", nCols, nRows, 1, GDT_Float32, NULL);
	geotiffDataset->SetGeoTransform(geoTransform);
	geotiffDataset->SetProjection(dem->GetProjectionRef());

	float *tab = (float *)CPLMalloc(sizeof(float) * (nCols * nRows));
	float *slope = (float *)CPLMalloc(sizeof(float) * ((nCols) * (nRows)));

	unsigned long long n = (nCols * nRows);
	cout << "Size of array: " << n << endl; 

	int read, save; 

	read = rasterBand->RasterIO(GF_Read, 0, 0, nCols, nRows, tab, nCols, nRows, GDT_Float32, 0, 0);

	begin = clock();
	for (r = 1; r < nRows - 1; r++)
	{
		for (c = 1; c < nCols - 1; c++)
		{
			p = ((tab[(r - 1) * nCols + (c + 1)] + 2 * tab[r * nCols + (c + 1)] + tab[(r + 1) * nCols + (c + 1)]) -
				 (tab[(r - 1) * nCols + (c - 1)] + 2 * tab[r * nCols + (c - 1)] + tab[(r + 1) * nCols + (c - 1)])) / 8;

			q = ((tab[(r + 1) * nCols + (c - 1)] + 2 * tab[(r + 1) * nCols + c] + tab[(r + 1) * nCols + (c + 1)]) -
				 (tab[(r - 1) * nCols + (c - 1)] + 2 * tab[(r - 1) * nCols + c] + tab[(r - 1) * nCols + (c + 1)])) / 8;

			slope[r * nCols + c] = atan(sqrt((p * p) + (q * q)));
		}
	}

	end = clock();
	double elapsed = double(end - begin) / CLOCKS_PER_SEC;

	cout << "Execution time: " << elapsed << " [s]";
	save = geotiffDataset->GetRasterBand(1)->RasterIO(GF_Write, 1, 1, nCols - 1, nRows - 1, slope, nCols - 1, nRows - 1, GDT_Float32, 0, 0);

	cout << "\n" << "Return of save: " << save;
	
	CPLFree(tab);
	CPLFree(slope);
	GDALClose(dem);
	GDALClose(geotiffDataset);
	GDALDestroyDriverManager();
	return 0;
}
