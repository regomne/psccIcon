photosop CC icon 资源解包程序
2013/09


这是个命令行程序：
psccIcon [-e|-p] <idx-file>

示例：
解包：psccIcon -e "D:\Photoshop CC\Resources\IconResources.idx"
	使用前需要保证PSIconsLowRes.dat和PSIconsHighRes.dat至少有一个在idx的同目录下。解包成功后会建立Low或High文件夹，里面是拆开的图片。
封包：psccIcon -p "D:\Photoshop CC\Resources\IconResources.idx"
	使用前须保证idx同目录下至少有Low或High文件夹其一。里面的所有png文件名和文件数量须和解包出来的一致。另图片的长宽最好也不要变化。封包后的文件名以“_”结尾，生成在同目录下（若已存在同名副本，会被覆盖）。
	
	
快捷用法：

解包：在同目录下没有High或Low文件夹时，直接把idx拖到psccIcon.exe上面。
封包：在同目录下至少有Hight或Low文件夹中的一个时，直接把idx拖到psccIcon.exe上面。