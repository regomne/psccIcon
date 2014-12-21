#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <Shlwapi.h>

#include <v8.h>

using namespace v8;

#define THROW_V8_NOMEM ThrowException(Exception::Error(String::New("No mem")))

#define THROW_V8_ERROR(str) ThrowException(Exception::Error(String::NewFromUtf8(g_Isolate,(str))))
#define THROW_V8_TYPEERROR(str)\
	ThrowException(Exception::TypeError(String::NewFromUtf8(g_Isolate,(str))))

Isolate* g_Isolate;

void ParseStrt(const v8::FunctionCallbackInfo<v8::Value>& infos)
{
	HandleScope handle_scope(g_Isolate);

	String::Utf8Value str(infos[0]->ToString());

	if(!*str||str.length()!=0xa0*2)
	{
		THROW_V8_ERROR("Can't cvt str");
		return;
	}

	auto arr=v8::Array::New(40);
	char tocvt[]="0xXXXXXXXX";
	char* p=*str;
	for(int i=0;i<0xa0/4;i++)
	{
		int val;
		memcpy(&tocvt[2],p,8);
		p+=8;
		auto ret=StrToIntExA(tocvt,STIF_SUPPORT_HEX,&val);
		if(!ret)
		{
			THROW_V8_ERROR("Can't cvt hex number");
			return;
		}
		arr->Set(i,Number::New(g_Isolate,(double)val));
	}
	infos.GetReturnValue().Set(arr);
}

void ExistFile(const FunctionCallbackInfo<Value>& infos)
{
	HandleScope scope(g_Isolate);
	String::Value str(infos[0]->ToString());

	if(!*str)
	{
		THROW_V8_ERROR("Can't cvt to String");
		return;
	}

	infos.GetReturnValue().Set((bool)(GetFileAttributes((wchar_t*)*str)!=INVALID_FILE_ATTRIBUTES));
}

void WriteToPng(const FunctionCallbackInfo<Value>& infos)
{
	HandleScope scope(g_Isolate);
	String::Value newpath(infos[0]->ToString());
	if(!*newpath)
	{
		THROW_V8_ERROR("Can't cvt to string");
		return;
	}
	String::Value dat(infos[1]->ToString());
	if(!*dat)
	{
		THROW_V8_ERROR("Can't cvt to string");
		return;
	}
	int offset=(int)(infos[2]->ToNumber()->Value());
	int size=(int)(infos[3]->ToNumber()->Value());

	auto hf=CreateFile((wchar_t*)*dat,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hf==INVALID_HANDLE_VALUE)
	{
		THROW_V8_ERROR("Can't open dat file!");
		return;
	}
	SetFilePointer(hf,offset,0,FILE_BEGIN);
	auto buff=new BYTE[size];
	DWORD temp;
	ReadFile(hf,buff,size,&temp,0);
	CloseHandle(hf);
	if(temp!=size)
	{
		THROW_V8_ERROR("Can't read file");
		return;
	}

	hf=CreateFile((wchar_t*)*newpath,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if(hf==INVALID_HANDLE_VALUE)
	{
		THROW_V8_ERROR("Can't write to new file");
		return;
	}
	WriteFile(hf,buff,size,&temp,0);
	CloseHandle(hf);
	delete[] buff;
}

void CreateFile1(const FunctionCallbackInfo<Value>& infos)
{
	String::Value path(infos[0]->ToString());
	
	auto hf=CreateFile((wchar_t*)*path,GENERIC_ALL,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if(hf!=INVALID_HANDLE_VALUE)
	{
		DWORD temp;
		WriteFile(hf,"fdra",4,&temp,0);
	}
	
	infos.GetReturnValue().Set((int32_t)hf);
}

void WriteFile1(const FunctionCallbackInfo<Value>& infos)
{
	auto hf=(HANDLE)(infos[0]->ToUint32()->Value());
	auto offset=(infos[1]->ToUint32()->Value());
	String::Value png(infos[2]->ToString());

	auto hpng=CreateFile((wchar_t*)*png,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hpng==INVALID_HANDLE_VALUE)
	{
		infos.GetReturnValue().Set((int32_t)-1);
		return;
	}
	int size=GetFileSize(hpng,0);
	auto buff=new char[size];
	DWORD temp;
	ReadFile(hpng,buff,size,&temp,0);
	CloseHandle(hpng);

	SetFilePointer(hf,offset,0,FILE_BEGIN);
	WriteFile(hf,buff,size,&temp,0);
	
	infos.GetReturnValue().Set((uint32_t)temp);
}

void CreateDir(const FunctionCallbackInfo<Value>& infos)
{
	String::Value path(infos[0]->ToString());
	auto ret=CreateDirectory((wchar_t*)*path,0);
	infos.GetReturnValue().Set((bool)(ret!=0));
}

void CloseFile(const FunctionCallbackInfo<Value>& infos)
{
	auto hf=(HANDLE)(infos[0]->ToUint32()->Value());
	CloseHandle(hf);
}

void WriteTxt(const FunctionCallbackInfo<Value>& infos)
{
	String::Value txt(infos[0]->ToString());
	String::Utf8Value content(infos[1]->ToString());
	auto hf=CreateFile((wchar_t*)*txt,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if(hf==INVALID_HANDLE_VALUE)
	{
		THROW_V8_ERROR("Can't open txt");
		return;
	}

	DWORD temp;
	WriteFile(hf,*content,content.length(),&temp,0);
	CloseHandle(hf);
}

// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile1(wchar_t* name) 
{
	auto hf=CreateFile(name,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hf==INVALID_HANDLE_VALUE)
		return Handle<String>();

	int size=GetFileSize(hf,0);
	auto buff=new BYTE[size];
	DWORD temp;
	ReadFile(hf,buff,size,&temp,0);
	CloseHandle(hf);
	if(temp!=size)
		return Handle<String>();

	auto s=String::NewFromUtf8(g_Isolate,(char*)buff,String::kNormalString,size);
	delete[] buff;
	return s;
}

void Read(const v8::FunctionCallbackInfo<v8::Value>& args) {
	if (args.Length() != 1) {
		THROW_V8_ERROR("Invalid parameters");
		return;
	}
	v8::String::Value file(args[0]);
	if (*file == NULL) {
		THROW_V8_ERROR("Can't cvt to string");
		return;
	}
	v8::Handle<v8::String> source = ReadFile1((wchar_t*)*file);
	if (source.IsEmpty()) {
		THROW_V8_ERROR("Error loading file");
		return;
	}
	args.GetReturnValue().Set(source);
}

char g_string[1000];
// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
	auto cstr=*value;
	if(!cstr)
		return "<string conversion failed>";
	int len=MultiByteToWideChar(CP_UTF8,0,cstr,-1,0,0);
	if(len!=0)
	{
		auto ss=new wchar_t[len];
		int nlen=MultiByteToWideChar(CP_UTF8,0,cstr,-1,ss,len);
		len=WideCharToMultiByte(936,0,ss,nlen,0,0,0,0);
		if(len>1000)
			len=1000;
		WideCharToMultiByte(936,0,ss,nlen,g_string,len,0,0);
		delete[] ss;
	}
	else
	{
		return *value;
	}
	return g_string;
}

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope(args.GetIsolate());
		if (first) {
			first = false;
		} else {
			printf(" ");
		}
		v8::String::Utf8Value str(args[i]);
		const char* cstr = ToCString(str);
		printf("%s", cstr);
	}
//	printf("\n");
	fflush(stdout);
}

v8::Handle<v8::Context> CreateShellContext(v8::Isolate* isolate) {
	// Create a template for the global object.
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
	// Bind the global 'print' function to the C++ Print callback.
	global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
	// Bind the global 'read' function to the C++ Read callback.
	global->Set(v8::String::New("read"), v8::FunctionTemplate::New(Read));

	global->Set(String::NewFromUtf8(g_Isolate,"existFile"),FunctionTemplate::New(ExistFile));
	global->Set(String::NewFromUtf8(g_Isolate,"parseStrt"),FunctionTemplate::New(ParseStrt));
	global->Set(String::NewFromUtf8(g_Isolate,"writePng"),FunctionTemplate::New(WriteToPng));
	global->Set(String::NewFromUtf8(g_Isolate,"createFile"),FunctionTemplate::New(CreateFile1));
	global->Set(String::NewFromUtf8(g_Isolate,"writeFile"),FunctionTemplate::New(WriteFile1));
	global->Set(String::NewFromUtf8(g_Isolate,"closeFile"),FunctionTemplate::New(CloseFile));
	global->Set(String::NewFromUtf8(g_Isolate,"writeTxt"),FunctionTemplate::New(WriteTxt));
	global->Set(String::NewFromUtf8(g_Isolate,"makeDir"),FunctionTemplate::New(CreateDir));

	return v8::Context::New(isolate, NULL, global);;
}

void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
	v8::HandleScope handle_scope(isolate);
	v8::String::Utf8Value exception(try_catch->Exception());
	const char* exception_string = ToCString(exception);
	v8::Handle<v8::Message> message = try_catch->Message();
	if (message.IsEmpty()) {
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		fprintf(stderr, "%s\n", exception_string);
	} else {
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(message->GetScriptResourceName());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber();
		fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
		// Print line of source code.
		v8::String::Utf8Value sourceline(message->GetSourceLine());
		const char* sourceline_string = ToCString(sourceline);
		fprintf(stderr, "%s\n", sourceline_string);
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++) {
			fprintf(stderr, " ");
		}
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++) {
			fprintf(stderr, "^");
		}
		fprintf(stderr, "\n");
		v8::String::Utf8Value stack_trace(try_catch->StackTrace());
		if (stack_trace.length() > 0) {
			const char* stack_trace_string = ToCString(stack_trace);
			fprintf(stderr, "%s\n", stack_trace_string);
		}
	}
}

bool ExecuteString(v8::Isolate* isolate,
	v8::Handle<v8::String> source,
	v8::Handle<v8::Value> name,
	bool print_result,
	bool report_exceptions) {
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch;
		v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
		if (script.IsEmpty()) {
			// Print errors that happened during compilation.
			if (report_exceptions)
				ReportException(isolate, &try_catch);
			return false;
		} else {
			v8::Handle<v8::Value> result = script->Run();
			if (result.IsEmpty()) {
				// Print errors that happened during execution.
				if (report_exceptions)
					ReportException(isolate, &try_catch);
				return false;
			} else {
				if (print_result && !result->IsUndefined()) {
					// If all went well and the result wasn't undefined then print
					// the returned value.
					v8::String::Utf8Value str(result);
					const char* cstr = ToCString(str);
					printf("%s\n", cstr);
				}
				return true;
			}
		}
}

wchar_t ExePath[2000];
int main()
{
	auto cmdLine=GetCommandLine();
	int argc;
	auto argv=CommandLineToArgvW(cmdLine,&argc);

	int slen=GetModuleFileName(GetModuleHandle(0),ExePath,sizeof(ExePath));
	if(slen>1900)
	{
		printf("path too long!\n");
		goto _Ex;
	}
	wchar_t* p=&ExePath[slen-1];
	while(*p!=L'\\')
		p--;
	wcscpy(p+1,L"program.js");
	
	g_Isolate = v8::Isolate::GetCurrent();
	{
		v8::HandleScope handle_scope(g_Isolate);

		v8::Handle<v8::Context> context = CreateShellContext(g_Isolate);
		if (context.IsEmpty()) {
			fprintf(stderr, "Error creating context\n");
			goto _Ex;
		}
		context->Enter();

		auto src=ReadFile1(ExePath);
		if(src.IsEmpty())
		{
			printf("Can't load program.js!\n");
			goto _Ex;
		}
		auto name=String::NewFromUtf8(g_Isolate,"ohehe");
		ExecuteString(g_Isolate,src,name,false,true);

		Handle<Value> arg[20];
		if(argc>20)
			argc=20;
		for(int i=0;i<argc;i++)
			arg[i]=String::NewFromTwoByte(g_Isolate,(uint16_t*)argv[i]);

		auto global=context->Global();
		auto mainFunc=global->Get(String::NewFromUtf8(g_Isolate,"main"));
		if(mainFunc->IsUndefined() || !mainFunc->IsFunction())
			printf("Need main function\n");
		else
			mainFunc.As<Function>()->Call(global,argc,arg);

		context->Exit();
	}
	_Ex:
//	system("pause");
	getch();
	return 0;
}

BOOL ReadDibData(BITMAPINFOHEADER* bi,BYTE** dib)
{
	*dib=new BYTE[bi->biHeight+100];
	if(!*dib)
	{
		return FALSE;
	}

	BYTE* p;
	if(bi->biBitCount!=8)
	{
		DWORD size=bi->biWidth*bi->biHeight*(bi->biBitCount/8);
		memcpy(*dib,bi+1,size);
	}
}