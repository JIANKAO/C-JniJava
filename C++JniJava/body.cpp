#include "windows.h"  
#include "jni.h"  
#include <iostream>

//参考文章：
// https://blog.csdn.net/xlxxcc/article/details/51106721  //字符转换
// https://www.cnblogs.com/jaejaking/p/6840848.html       //实战
// https://www.cnblogs.com/andyliu1988/p/6041542.html     //实战

//转码输出
void u_out(JNIEnv* env, jclass java_class, const char* str);
//编码转换
char* jstringToChar(JNIEnv* env, jstring jstr);
jstring charTojstring(JNIEnv* env, const char* str);

//编译java程序cmd指令(别忘了改盘符和路径)：javac -classpath L:\ L:\JIANKAO_C++\代码\E临时测试\C++调用Java\C++JniJava\C++JniJava\test\Demo.java
int main()
{
	// 设置控制台输出为 UTF-8(这里有一个坑，这样设置虽然让java的字符转换正常在控制台显示了，但会让你直接打印的中文变为乱码，参考:u_out(JNIEnv* env, jclass java_class, const char* str))
	SetConsoleOutputCP(CP_UTF8);
	//定义一个函数指针，下面用来指向JVM中的JNI_CreateJavaVM函数  
	typedef jint(WINAPI* PFunCreateJavaVM)(JavaVM**, void**, void*);

	int res;
	JavaVMInitArgs vm_args;
	JavaVMOption options[3];
	JavaVM* jvm;
	JNIEnv* env;//这个就是最终的虚拟机管理员，他可以调用JAVA里的任何一个类
	/**************************************虚拟机的配置和运行****************************************/
	char value1[50]{ '\0' }, value2[50]{ '\0' }, value3[50]{ '\0' };
	strcpy_s(value1, "-Xshare:off\0");
	strcpy_s(value2, "-Djava.class.path=.;c:\\\0");
	strcpy_s(value3, "-verbose:NONE\0");
	/*设置初始化参数*/
	//disable JIT，这是JNI文档中的解释，具体意义不是很清楚 ，能取哪些值也不清楚。  
	//从JNI文档里给的示例代码中搬过来的  
	options[0].optionString = value1;
	//设置classpath，如果程序用到了第三方的JAR包，也可以在这里面包含进来  
	options[1].optionString = value2;
	//设置显示消息的类型，取值有gc、class和jni，如果一次取多个的话值之间用逗号格开，如-verbose:gc,class  
	//该参数可以用来观察C++调用JAVA的过程，设置该参数后，程序会在标准输出设备上打印调用的相关信息  
	options[2].optionString = value3;

	//设置版本号，版本号有JNI_VERSION_1_1，JNI_VERSION_1_2和JNI_VERSION_1_4  
	//选择一个根你安装的JRE版本最近的版本号即可，不过你的JRE版本一定要等于或者高于指定的版本号  
	vm_args.version = JNI_VERSION_20;
	vm_args.nOptions = 3;
	vm_args.options = options;
	//该参数指定是否忽略非标准的参数，如果填JNI_FLASE，当遇到非标准参数时，JNI_CreateJavaVM会返回JNI_ERR  
	vm_args.ignoreUnrecognized = JNI_TRUE;
	//加载JVM.DLL动态库  
	HINSTANCE hInstance = ::LoadLibrary(L"C:\\Program Files\\Java\\jdk-22\\jre\\bin\\server\\jvm.dll");
	if (hInstance == NULL)
	{
		return false;
	}
	//取得里面的JNI_CreateJavaVM函数指针  
	PFunCreateJavaVM funCreateJavaVM = (PFunCreateJavaVM)::GetProcAddress(hInstance, "JNI_CreateJavaVM");
	//调用JNI_CreateJavaVM创建虚拟机  
	res = (*funCreateJavaVM)(&jvm, (void**)&env, &vm_args);
	if (res < 0)
	{
		std::cout << "ERROR!" << std::endl;
		return -1;
	}
	else
	{
		std::cout << "finish:" << std::endl;
	}

	/****************************正式调用JAVA类***********************************/

	//查找test.Demo类，返回JAVA类的CLASS对象 
	jmethodID mid;
	jclass cls = env->FindClass("test/Demo");
	//测试项一:调用静态方法  getHelloWorld()
	if (cls != nullptr)
	{
		//方法ID获取(getHelloWorld是一个静态函数,所以用GetStaticMethodID)
		mid = env->GetStaticMethodID(cls, "getHelloWorld", "()Ljava/lang/String;");
		if (mid != nullptr)
		{
			//方法调用(如果上面有参数就从第二个参数开始一个一个填)//其实就相当于使用id对应的函数，java_str就是返回值，当然，这个函数没有形参所以只有（cls, mid）
			jstring java_str = (jstring)env->CallStaticObjectMethod(cls, mid);
			if (java_str != nullptr)
			{
				//字符转换：jstring -> char*  GetStringUTFChars(/*jstring要转换字符串*/,/*一般为nullptr*/)//不支持中文,但不用处理宽字符,调用ReleaseStringUTFChars()清理内存
				//字符转换：jstring -> char*  GetStringChars(/*jstring要转换字符串*/,/*一般为nullptr*/)//支持中文,但需要处理宽字符,调用ReleaseStringChars()清理内存
				const char* unicode_str = env->GetStringUTFChars(java_str, nullptr);
				if (unicode_str != nullptr)
				{
					// 打印或使用
					std::cout << unicode_str << std::endl;
					// 释放 Unicode 字符串
					env->ReleaseStringUTFChars(java_str, unicode_str);
				}
				else {
					u_out(env, cls, "字符串转换失败!-getHelloWorld");
				}
			}
			else {
				u_out(env, cls, "方法调用失败!-getHelloWorld");
			}
		}
		else {
			u_out(env, cls, "方法ID获取失败!-getHelloWorld");
		}
	}
	else {
		u_out(env, cls, "类加载失败!-getHelloWorld");
	}
	//测试项二:调用带参构造函数Demo(String msg)往类内的私有变量写值
	jobject demo{};//用来保存构造函数，方便下一个验证
	if (cls != nullptr)
	{
		//构造函数不是静态的所以要用GetMethodID
		mid = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;)V");
		if (mid != nullptr)
		{
			//这里有一个坑，直接使用NewStringUTF并不能正确录入,这很奇怪,这是编码的问题，之前拿C++写的管理工具开MC服务器也是，必须得手动处理编码问题。
			jstring java_str = charTojstring(env, "煎烤");
			//创建带参构造
			demo = env->NewObject(cls, mid, java_str);
		}
		else {
			u_out(env, cls, "方法ID获取失败!-Demo");
		}
	}
	else {
		u_out(env, cls, "类加载失败!-Demo");
	}
	//验证是否构造成功
	if (cls != nullptr)
	{
		mid = env->GetMethodID(cls, "getStrData", "()Ljava/lang/String;");
		if (mid != nullptr)
		{
			jstring java_str = (jstring)env->CallObjectMethod(demo, mid);
			const char* unicode_str = env->GetStringUTFChars(java_str, nullptr);
			if (unicode_str != nullptr)
			{
				// 打印或使用
				std::cout << unicode_str << std::endl;
				// 释放 Unicode 字符串
				env->ReleaseStringUTFChars(java_str, unicode_str);
			}
			else {
				u_out(env, cls, "字符串转换失败!-Demo");
			}
		}
		else {
			u_out(env, cls, "方法ID获取失败!-Demo");
		}
	}
	else {
		u_out(env, cls, "类加载失败!-Demo");
	}
	//测试项三:
	//根据类的CLASS对象获取该类的实例  
	//jobject obj = env->AllocObject(cls);//获取默认构造的实例
	jobject obj_1 = demo;//这里是使用了上一个测试新建的构造的实例
	if (cls != nullptr)
	{
		//这里（）里的多个签名是不需要;隔开的，这里的;只是String签名自带的，如果是三个int参数那就是(III)Ljava/lang/String;
		mid = env->GetMethodID(cls, "append", "(Ljava/lang/String;I)Ljava/lang/String;");
		if (mid != nullptr)
		{
			jstring java_str = charTojstring(env, "煎烤");
			jstring return_str = (jstring)env->CallObjectMethod(obj_1, mid, java_str, 10);
			const char* unicode_str = env->GetStringUTFChars(return_str, nullptr);
			if (unicode_str != nullptr)
			{
				// 打印或使用
				std::cout << unicode_str << std::endl;
				// 释放 Unicode 字符串
				env->ReleaseStringUTFChars(return_str, unicode_str);
			}
			else {
				u_out(env, cls, "字符串转换失败!-append");
			}
		}
		else {
			u_out(env, cls, "方法ID获取失败!-append");
		}
	}
	else {
		u_out(env, cls, "类加载失败!-append");
	}
	//测试项四:
	//异常处理
	//jobject obj_2 = env->AllocObject(cls);//获取默认构造的实例
	jobject obj_2 = demo;//这里是使用了上上一个测试新建的构造的实例
	if (cls != nullptr)
	{
		mid = env->GetMethodID(cls, "throwExcp", "()V");
		if (mid != nullptr)
		{
			env->CallVoidMethod(obj_2, mid);
			//获取异常信息
			jthrowable excp = 0;
			excp = env->ExceptionOccurred();
			if (excp)
			{
				jclass cls = env->GetObjectClass(excp);
				jmethodID mid = env->GetMethodID(cls, "toString", "()Ljava/lang/String;");
				jstring return_str = (jstring)env->CallObjectMethod(excp, mid);
				//转换字符并输出返回数据
				const char* unicode_str = env->GetStringUTFChars(return_str, nullptr);
				if (unicode_str != nullptr)
				{
					// 打印或使用
					std::cout << unicode_str << std::endl;
					// 释放 Unicode 字符串
					env->ReleaseStringUTFChars(return_str, unicode_str);
				}
				else
				{
					std::cout << "字符串转换失败!-throwExcp" << std::endl;
				}
				//清除异常状态
				env->ExceptionClear();
			}
		}
		else {
			u_out(env, cls, "方法ID获取失败!-throwExcp");
		}
	}
	else {
		u_out(env, cls, "类加载失败!-throwExcp");
	}
	//测试项:五
	//数组对象的构造和访问
	int int_array[] = { 1,2,3,4,5,6 };//尽量用jint，提高兼容性
	//这里计算元素个数的公式就是总大小除一个元素大小，long，long long都可以用
	jintArray array = env->NewIntArray(sizeof(int_array) / sizeof(*int_array));
	//设置数组从哪里开始填充，从哪里结束(这里是从0也就是从头开始，一直到整个数组结束)
	env->SetIntArrayRegion(array, 0, sizeof(int_array) / sizeof(*int_array), int_array);
	//开始传入
	jobject obj_3 = demo;
	mid = env->GetMethodID(cls, "setCounts", "([I)V");
	if (mid != nullptr)
	{
		env->CallVoidMethod(obj_3, mid, array);
	}
	//获取传入的数组
	mid = env->GetMethodID(cls, "getCounts", "()[I");
	if (mid != nullptr)
	{
		jintArray msg = (jintArray)env->CallObjectMethod(obj_3, mid, array);
		int len = env->GetArrayLength(msg);
		jint* elems = env->GetIntArrayElements(msg, 0);
		for (int i = 0; i < len; i++)
		{
			//cout << "下标: " << i << " 值为: " << elems[i] << endl;
			u_out(env, cls, "下标: "), std::cout << i, u_out(env, cls, " 值为: "), std::cout << elems[i] << std::endl;
		}
		env->ReleaseIntArrayElements(msg, elems, 0);
	}
	/*************************************善后处理**************************************/

	//销毁虚拟机并释放动态库
	jvm->DestroyJavaVM();
	::FreeLibrary(hInstance);
	return 0;
}

//utf8/16转换成gb2312(用完记得free释放内存!尽量不要使用该函数，不然忘记free会导致内存泄露)
char* jstringToChar(JNIEnv* env, jstring jstr)
{
	int length = env->GetStringLength(jstr);
	const jchar* jcstr = env->GetStringChars(jstr, 0);
	char* rtn = (char*)malloc(length * 2 + 1);
	int size = 0;
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)jcstr, length, rtn,
		(length * 2 + 1), NULL, NULL);
	if (size <= 0)
		return NULL;
	env->ReleaseStringChars(jstr, jcstr);
	rtn[size] = 0;
	return rtn;
}

//gb2312转换成utf8/16
jstring charTojstring(JNIEnv* env, const char* str)
{
	jstring rtn = 0;
	int slen = (int)strlen(str);
	unsigned short* buffer = 0;
	if (slen == 0)
		rtn = env->NewStringUTF(str);
	else {
		int length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str, slen, NULL, 0);
		buffer = (unsigned short*)malloc(length * 2 + 1);
		if (MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str, slen, (LPWSTR)buffer, length) > 0)
			rtn = env->NewString((jchar*)buffer, length);
		// 释放内存
		free(buffer);
	}
	return rtn;
}

void u_out(JNIEnv* env, jclass java_class, const char* str)
{
	//采用demo转换
	//jobject obj = env->AllocObject(java_class);//获取默认构造的实例
	//jmethodID mid;
	//if (java_class != nullptr)
	//{
	//	//这里（）里的多个签名是不需要;隔开的，这里的;只是String签名自带的，如果是三个int形参那就是(III)Ljava/lang/String;
	//	mid = env->GetMethodID(java_class, "setStrData", "(Ljava/lang/String;)Ljava/lang/String;");
	//	if (mid != nullptr)
	//	{
	//		jstring java_str = charTojstring(env, str);
	//		jstring return_str = (jstring)env->CallObjectMethod(obj, mid, java_str);
	//		const char* unicode_str = env->GetStringUTFChars(return_str, nullptr);
	//		if (unicode_str != nullptr)
	//		{
	//			// 打印或使用
	//			std::cout << unicode_str;
	//			// 释放 Unicode 字符串
	//			env->ReleaseStringUTFChars(return_str, unicode_str);
	//			return;
	//		}
	//		
	//	}
	//	
	//}

	//直接转换
	jstring java_str = charTojstring(env, str);
	const char* unicode_str = env->GetStringUTFChars(java_str, nullptr);
	if (unicode_str != nullptr)
	{
		// 打印或使用
		std::cout << unicode_str;
		// 释放 Unicode 字符串
		env->ReleaseStringUTFChars(java_str, unicode_str);
		return;
	}

	std::cout << "ERROR_2" << std::endl;
}