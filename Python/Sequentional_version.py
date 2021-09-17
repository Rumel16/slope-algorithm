from osgeo import gdal
import numpy as np
import sys
import time

def Slope(rows, cols, dem, tab):
    for c in range(1,rows-2):
        for r in range(1, cols-2):
            p = ((dem[r-1][c+1] + 2*dem[r][c+1] + dem[r+1][c+1]) - (dem[r-1][c-1] + 2*dem[r][c-1] + dem[r+1][c-1]))/8

            q = ((dem[r+1][c+1] + 2*dem[r+1][c] + dem[r+1][c-1]) - (dem[r-1][c+1] + 2*dem[r-1][c] + dem[r-1][c-1]))/8

            tab[r][c] = np.arctan(np.sqrt(p*p+q*q))



gdal.UseExceptions()
gdal.AllRegister()

try:
    ds = gdal.Open('40x40.tif')
except RuntimeError:
    sys.exit("Błąd podczas otwierania pliku wejściowego")

band = ds.GetRasterBand(1)
dem = band.ReadAsArray()
rows = dem.shape[1]
cols = dem.shape[0]
print("nRows: ", rows)
print("nCols: ", cols)
print("Rozmiar tablicy: ", rows*cols)

driver = gdal.GetDriverByName('GTiff')
dataset = driver.Create('slopePy.tif',rows-1, cols-1, 1,gdal.GDT_Float32)

geotrans=ds.GetGeoTransform()
proj=ds.GetProjection() 
dataset.SetGeoTransform(geotrans)
dataset.SetProjection(proj)

tab = np.zeros((cols-1,rows-1))

start = time.time()

Slope(rows, cols, dem, tab)

end = time.time()

outds = dataset.GetRasterBand(1)
outds.WriteArray(tab)
outds.SetNoDataValue(np.nan)
outds.FlushCache()
ds.FlushCache()
outds = None
ds = None


print("Czas wykonywania: ",end - start, " [s]")

