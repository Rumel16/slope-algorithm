from osgeo import gdal
import numpy as np
from numba import jit, cuda
import sys
import math
import time

@cuda.jit
def Slope(dem, tab):
    c,r=cuda.grid(2)
    if c>0 and c<dem.shape[0] -1 and r>0 and r<dem.shape[1]-1:

        p = ((dem[c+1,r-1] + 2*dem[c+1,r] + dem[c+1,r+1]) - (dem[c-1,r-1] + 2*dem[c-1,r] + dem[c-1,r+1]))/8

        q = ((dem[c+1,r+1] + 2*dem[c,r+1] + dem[c-1,r+1]) - (dem[c+1,r-1] + 2*dem[c,r-1] + dem[c-1,r-1]))/8

        tab[c,r] = math.atan(math.sqrt(p*p+q*q))



gdal.UseExceptions()
gdal.AllRegister()


try:
    ds = gdal.Open('40x40.tif')
except RuntimeError:
    sys.exit("Błąd podczas otwierania pliku wejściowego")

band = ds.GetRasterBand(1)
dem = band.ReadAsArray()
rows = ds.RasterYSize
cols = ds.RasterXSize
print("nRows: ", rows)
print("nCols: ", cols)
print("Rozmiar tablicy: ", rows*cols)

driver = gdal.GetDriverByName('GTiff')
dataset = driver.Create('slopePy.tif',cols, rows, 1,gdal.GDT_Float32)

geotrans=ds.GetGeoTransform()  
proj=ds.GetProjection() 
dataset.SetGeoTransform(geotrans)
dataset.SetProjection(proj)

tab = np.zeros_like(dem)

blockSize = 32
block=(blockSize,blockSize)
gsize = (rows//blockSize+(1 if rows%blockSize>0 else 0), cols//blockSize + (1 if cols%blockSize>0 else 0))


start = time.time()

Slope[gsize, block](dem, tab)

end = time.time()

outds = dataset.GetRasterBand(1)
outds.WriteArray(tab)
outds.SetNoDataValue(np.nan)
outds.FlushCache()
ds.FlushCache()
outds = None
ds = None

print("Czas wykonywania: ",end - start, " [s]")

