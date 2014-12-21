//Extract the icon files

function DeserializeIconInfo(icons)
{
	for(var key in icons)
	{
		if(icons[key].length!=0xa0*2)
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
				pics[i]={offset:s[i*2],size:s[i*2+1]};
			return pics;
		};
		
		info.low.pics=GetPicInfo(strt.slice(4,20));
		info.high.pics=GetPicInfo(strt.slice(24,40));
		
		icons[key]=info;
	}
}

function SerializeIconInfo(icons)
{
    if(typeof(String.prototype.rjust)!='function')
    {
        String.prototype.rjust=function(cnt,char)
        {
            if(typeof(char)!='string' || char.length!=1)
                char=' ';
            if(this.length>=cnt)
                return this;
            cnt-=this.length;
            left='';
            for(i=0;i<cnt;i++)
                left+=char;
            return left+this;
        };
    }
    for(var key in icons)
    {
        info=icons[key];
        var s=[];
        function seriLow(inf)
        {
            var s=[inf.width,inf.height,inf.x,inf.y];
            for(i=0;i<8;i++)
            {
                s.push(inf.pics[i].offset);
                s.push(inf.pics[i].size);
            }
            return s;
        }
        s=s.concat(seriLow(info.low));
        s=s.concat(seriLow(info.high));
        str='';
        for(j=0;j<40;j++)
            str+=s[j].toString(16).toUpperCase().rjust(8,'0');
        icons[key]=str;
    }
}

function repr(ls,dic,indent)
{
    if(typeof(String.prototype.repeat)!='function')
    {
        String.prototype.repeat=function(cnt)
        {
            s=new Array(cnt);
            for(i=0;i<cnt;i++)
                s[i]=this;
            return s.join('');
        };
    }
    if(typeof(indent)!='number')
        indent=1;

    for(var key in dic)
    {
        elecnt=0;
        elem=dic[key];
        if(typeof(elem)=='object')
        {
            for(var k in elem)
                elecnt++;
        }
        if(elecnt==0)
        {
            ls.push('\t'.repeat(indent)+key+': "'+elem+'",');
        }
        else
        {
            ls.push('\t'.repeat(indent)+key+': {');
            repr(ls,elem,indent+1);
            ls.push('\t'.repeat(indent)+'},');
        }
    }
    if(ls.length>0)
    {
        ls[ls.length-1]=ls[ls.length-1].slice(0,-1);
    }
}

function Ext(path,name)
{
    print('Reading and parsing '+name+'...');
	eval('dic='+read(path+name));
	if(dic.Name!="Photoshop Icon Source File" || dic.Version!='1.0')
		throw 'only support ver 1.0';
	
	DeserializeIconInfo(dic.Icons);

	var hasHigh=false;
	var hasLow=false;
	if(existFile(path+dic.HighResolutionDataFile))
    {
		hasHigh=true;
        makeDir(path+'High');
        print(dic.HighResolutionDataFile,'found.');
    }
		
	if(existFile(path+dic.LowResolutionDataFile))
    {
		hasLow=true;
        makeDir(path+'Low');
        print(dic.LowResolutionDataFile,'found.');
    }
	
	if(!hasHigh && !hasLow)
		throw 'need LowResolutionDataFile or HighResolutionDataFile';
	
	print("Extracting files...");
	for(key in dic.Icons)
	{
		if(hasHigh)
		{
			hi=dic.Icons[key].high;
			for(i=0;i<8;i++)
			{
				if(hi.pics[i].offset!=0)
					writePng(path+'High\\'+key+'_s'+i+'.png',path+dic.HighResolutionDataFile,
                        hi.pics[i].offset,hi.pics[i].size);
			}
		}
		if(hasLow)
		{
			hi=dic.Icons[key].low;
			for(i=0;i<8;i++)
			{
				if(hi.pics[i].offset!=0)
					writePng(path+'Low\\'+key+'_s'+i+'.png',path+dic.LowResolutionDataFile,
                        hi.pics[i].offset,hi.pics[i].size);
			}
		}
	}
    print("All complete.");

}

function Pack(path,name)
{
    print('Reading and parsing '+name+'...');
	eval('dic='+read(path+name));
	if(dic.Name!="Photoshop Icon Source File" || dic.Version!='1.0')
		throw 'only support ver 1.0';
	
	DeserializeIconInfo(dic.Icons);

	var hasHigh=false;
	var hasLow=false;
	var fsHigh,fsLow;

	if(existFile(path+'High'))
    {
		hasHigh=true;
        fsHigh=createFile(path+dic.HighResolutionDataFile.split('.').join('_.'));
        if(fsHigh==-1)
            throw "Can't create dat file";
        print('High folder found and dat file created.');
    }
		
	if(existFile(path+'Low'))
    {
		hasLow=true;
        fsLow=createFile(path+dic.LowResolutionDataFile.split('.').join('_.'));
        if(fsLow==-1)
            throw "Can't create dat file";
        print('Low folder found and dat file created.');
    }

	if(!hasHigh && !hasLow)
		throw 'need Low or High folder';


    if(hasHigh)
    {
        print('Packing high dat...');
        curOff=4;
        for(key in dic.Icons)
        {
            hi=dic.Icons[key].high;
            for(i=0;i<8;i++)
            {
                if(hi.pics[i].offset!=0)
                {
                    pngName=path+'High\\'+key+'_s'+i+'.png';
                    size=writeFile(fsHigh,curOff,pngName);
                    if(size==-1)
                        throw "Can't open "+pngName;
                    hi.pics[i].offset=curOff;
                    hi.pics[i].size=size;
                    curOff+=size;
                }
            }
        }
        closeFile(fsHigh);
    }
    if(hasLow)
    {
        print('Packing low dat...');
        curOff=4;
        for(key in dic.Icons)
        {
            hi=dic.Icons[key].low;
            for(i=0;i<8;i++)
            {
                if(hi.pics[i].offset!=0)
                {
                    pngName=path+'Low\\'+key+'_s'+i+'.png';
                    size=writeFile(fsLow,curOff,pngName);
                    if(size==-1)
                        throw "Can't open "+pngName;
                    hi.pics[i].offset=curOff;
                    hi.pics[i].size=size;
                    curOff+=size;
                }
            }
        }
        closeFile(fsLow);
    }

    print('Generating index...');
    SerializeIconInfo(dic.Icons);
    ls=['{'];
    repr(ls,dic,1);
    ls.push('}');
    print('Writing idx file...');
    writeTxt(path+name.split('.').join('_.'),ls.join('\r\n'));
    print('All complete.');
}

function main(exeName)
{
    print('Photoshop CC Icon Resource Extractor/Packer v1.0 by AmaF\n');

	if(arguments.length==1)
	{
		print('\n\tUsage: '+exeName.split('\\').slice(-1)+' [-e|-p] <idx-file>');
		return;
	}
//    arguments[1]='-p';
//    arguments[2]='IconResources.idx';
//    
    var arg1=arguments[1].toString();
    var idxName;
    var isExt;
    if(arg1=='-e')
    {
        isExt=true;
        idxName=arguments[2].toString();
    }
    else if(arg1=='-p')
    {
        isExt=false;
        idxName=arguments[2].toString();
    }
    else
    {
        idxName=arg1;
        isExt=undefined;
    }

    var idxPath='';
    if(idxName.search(/\\|\//)!=-1)
    {
        s=idxName.split(/\\|\//);
        idxPath=s.slice(0,-1).join('\\');
        if(idxPath.length!=0)
            idxPath+='\\';

        idxName=s.slice(-1)[0];
    }

    if(isExt==undefined)
    {
        if(existFile(idxPath+"High") || existFile(idxPath+"Low"))
            isExt=false;
        else
            isExt=true;
    }

    try
    {
        if(isExt)
        {
            Ext(idxPath,idxName);
        }
        else
        {
            Pack(idxPath,idxName);
        }
    }
    catch(e)
    {
        print(e);
    }
}