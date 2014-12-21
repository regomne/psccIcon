//Extract the icon files

function ConvertIconStruct(icons)
{
	for(var key in icons)
	{
		if(icons[key].length!=0xa0)
			throw "unrecoginized format";
		strt=parseStrt(icons[key]);
		info={
			low:{
				width:strt[0],
				height:strt[1],
				x:strt[2],
				y:strt[3],
				
			},
			high:{
				width:strt[20],
				height:strt[21],
				x:strt[22],
				y:strt[23],
			},
		};
		
		function GetPicInfo(s)
		{
			pics=[];
			for(i=0;i<8;i++)
			{
				pics[i].offset=s[i*2];
				pics[i].size=s[i*2+1];
			}
			return pics;
		};
		
		info.low.pics=GetPicInfo(s.slice(4,20));
		info.high.pics=GetPicInfo(s.slice(24,40));
		
		icons[key]=info;
	}
}

function Ext(name)
{
	eval('dic='+read(name));
	if(dic.name!="Photoshop Icon Source File" || dic.Version!='1.0')
		throw 'only support ver 1.0';
	
	var hasHigh=false;
	var hasLow=false;
	var fsHigh,fsLow;
	if(existFile(dic.HighResolutionDataFile))
		hasHigh=true;
		
	if(existFile(dic.LowResolutionDataFile))
		hasLow=true;
	
	if(!hasHigh && !hasLow)
		throw 'need LowResolutionDataFile or HighResolutionDataFile';
		
	ConvertIconStruct(dic.Icons);
	
	for(key in dic.Icons)
	{
		if(hasHigh)
		{
			hi=dic.Icons[key].high;
			for(i=0;i<8;i++)
			{
				if(hi.pics[i]!=undefined)
					writeFile('High\\'+key+'_s'+i+'.png',dic.HighResolutionDataFile,hi.pics[i].offset,hi.pics[i].size);
			}
		}
	}

}