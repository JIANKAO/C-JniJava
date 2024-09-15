#include "windows.h"  
#include "jni.h"  
#include <iostream>

//�ο����£�
// https://blog.csdn.net/xlxxcc/article/details/51106721  //�ַ�ת��
// https://www.cnblogs.com/jaejaking/p/6840848.html       //ʵս
// https://www.cnblogs.com/andyliu1988/p/6041542.html     //ʵս

//ת�����
void u_out(JNIEnv* env, jclass java_class, const char* str);
//����ת��
char* jstringToChar(JNIEnv* env, jstring jstr);
jstring charTojstring(JNIEnv* env, const char* str);

//����java����cmdָ��(�����˸��̷���·��)��javac -classpath L:\ L:\JIANKAO_C++\����\E��ʱ����\C++����Java\C++JniJava\C++JniJava\test\Demo.java
int main()
{
	// ���ÿ���̨���Ϊ UTF-8(������һ���ӣ�����������Ȼ��java���ַ�ת�������ڿ���̨��ʾ�ˣ���������ֱ�Ӵ�ӡ�����ı�Ϊ���룬�ο�:u_out(JNIEnv* env, jclass java_class, const char* str))
	SetConsoleOutputCP(CP_UTF8);
	//����һ������ָ�룬��������ָ��JVM�е�JNI_CreateJavaVM����  
	typedef jint(WINAPI* PFunCreateJavaVM)(JavaVM**, void**, void*);

	int res;
	JavaVMInitArgs vm_args;
	JavaVMOption options[3];
	JavaVM* jvm;
	JNIEnv* env;//����������յ����������Ա�������Ե���JAVA����κ�һ����
	/**************************************����������ú�����****************************************/
	char value1[50]{ '\0' }, value2[50]{ '\0' }, value3[50]{ '\0' };
	strcpy_s(value1, "-Xshare:off\0");
	strcpy_s(value2, "-Djava.class.path=.;c:\\\0");
	strcpy_s(value3, "-verbose:NONE\0");
	/*���ó�ʼ������*/
	//disable JIT������JNI�ĵ��еĽ��ͣ��������岻�Ǻ���� ����ȡ��ЩֵҲ�������  
	//��JNI�ĵ������ʾ�������а������  
	options[0].optionString = value1;
	//����classpath����������õ��˵�������JAR����Ҳ�������������������  
	options[1].optionString = value2;
	//������ʾ��Ϣ�����ͣ�ȡֵ��gc��class��jni�����һ��ȡ����Ļ�ֵ֮���ö��Ÿ񿪣���-verbose:gc,class  
	//�ò������������۲�C++����JAVA�Ĺ��̣����øò����󣬳�����ڱ�׼����豸�ϴ�ӡ���õ������Ϣ  
	options[2].optionString = value3;

	//���ð汾�ţ��汾����JNI_VERSION_1_1��JNI_VERSION_1_2��JNI_VERSION_1_4  
	//ѡ��һ�����㰲װ��JRE�汾����İ汾�ż��ɣ��������JRE�汾һ��Ҫ���ڻ��߸���ָ���İ汾��  
	vm_args.version = JNI_VERSION_20;
	vm_args.nOptions = 3;
	vm_args.options = options;
	//�ò���ָ���Ƿ���ԷǱ�׼�Ĳ����������JNI_FLASE���������Ǳ�׼����ʱ��JNI_CreateJavaVM�᷵��JNI_ERR  
	vm_args.ignoreUnrecognized = JNI_TRUE;
	//����JVM.DLL��̬��  
	HINSTANCE hInstance = ::LoadLibrary(L"C:\\Program Files\\Java\\jdk-22\\jre\\bin\\server\\jvm.dll");
	if (hInstance == NULL)
	{
		return false;
	}
	//ȡ�������JNI_CreateJavaVM����ָ��  
	PFunCreateJavaVM funCreateJavaVM = (PFunCreateJavaVM)::GetProcAddress(hInstance, "JNI_CreateJavaVM");
	//����JNI_CreateJavaVM���������  
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

	/****************************��ʽ����JAVA��***********************************/

	//����test.Demo�࣬����JAVA���CLASS���� 
	jmethodID mid;
	jclass cls = env->FindClass("test/Demo");
	//������һ:���þ�̬����  getHelloWorld()
	if (cls != nullptr)
	{
		//����ID��ȡ(getHelloWorld��һ����̬����,������GetStaticMethodID)
		mid = env->GetStaticMethodID(cls, "getHelloWorld", "()Ljava/lang/String;");
		if (mid != nullptr)
		{
			//��������(��������в����ʹӵڶ���������ʼһ��һ����)//��ʵ���൱��ʹ��id��Ӧ�ĺ�����java_str���Ƿ���ֵ����Ȼ���������û���β�����ֻ�У�cls, mid��
			jstring java_str = (jstring)env->CallStaticObjectMethod(cls, mid);
			if (java_str != nullptr)
			{
				//�ַ�ת����jstring -> char*  GetStringUTFChars(/*jstringҪת���ַ���*/,/*һ��Ϊnullptr*/)//��֧������,�����ô�����ַ�,����ReleaseStringUTFChars()�����ڴ�
				//�ַ�ת����jstring -> char*  GetStringChars(/*jstringҪת���ַ���*/,/*һ��Ϊnullptr*/)//֧������,����Ҫ������ַ�,����ReleaseStringChars()�����ڴ�
				const char* unicode_str = env->GetStringUTFChars(java_str, nullptr);
				if (unicode_str != nullptr)
				{
					// ��ӡ��ʹ��
					std::cout << unicode_str << std::endl;
					// �ͷ� Unicode �ַ���
					env->ReleaseStringUTFChars(java_str, unicode_str);
				}
				else {
					u_out(env, cls, "�ַ���ת��ʧ��!-getHelloWorld");
				}
			}
			else {
				u_out(env, cls, "��������ʧ��!-getHelloWorld");
			}
		}
		else {
			u_out(env, cls, "����ID��ȡʧ��!-getHelloWorld");
		}
	}
	else {
		u_out(env, cls, "�����ʧ��!-getHelloWorld");
	}
	//�������:���ô��ι��캯��Demo(String msg)�����ڵ�˽�б���дֵ
	jobject demo{};//�������湹�캯����������һ����֤
	if (cls != nullptr)
	{
		//���캯�����Ǿ�̬������Ҫ��GetMethodID
		mid = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;)V");
		if (mid != nullptr)
		{
			//������һ���ӣ�ֱ��ʹ��NewStringUTF��������ȷ¼��,������,���Ǳ�������⣬֮ǰ��C++д�Ĺ����߿�MC������Ҳ�ǣ�������ֶ�����������⡣
			jstring java_str = charTojstring(env, "�忾");
			//�������ι���
			demo = env->NewObject(cls, mid, java_str);
		}
		else {
			u_out(env, cls, "����ID��ȡʧ��!-Demo");
		}
	}
	else {
		u_out(env, cls, "�����ʧ��!-Demo");
	}
	//��֤�Ƿ���ɹ�
	if (cls != nullptr)
	{
		mid = env->GetMethodID(cls, "getStrData", "()Ljava/lang/String;");
		if (mid != nullptr)
		{
			jstring java_str = (jstring)env->CallObjectMethod(demo, mid);
			const char* unicode_str = env->GetStringUTFChars(java_str, nullptr);
			if (unicode_str != nullptr)
			{
				// ��ӡ��ʹ��
				std::cout << unicode_str << std::endl;
				// �ͷ� Unicode �ַ���
				env->ReleaseStringUTFChars(java_str, unicode_str);
			}
			else {
				u_out(env, cls, "�ַ���ת��ʧ��!-Demo");
			}
		}
		else {
			u_out(env, cls, "����ID��ȡʧ��!-Demo");
		}
	}
	else {
		u_out(env, cls, "�����ʧ��!-Demo");
	}
	//��������:
	//�������CLASS�����ȡ�����ʵ��  
	//jobject obj = env->AllocObject(cls);//��ȡĬ�Ϲ����ʵ��
	jobject obj_1 = demo;//������ʹ������һ�������½��Ĺ����ʵ��
	if (cls != nullptr)
	{
		//�������Ķ��ǩ���ǲ���Ҫ;�����ģ������;ֻ��Stringǩ���Դ��ģ����������int�����Ǿ���(III)Ljava/lang/String;
		mid = env->GetMethodID(cls, "append", "(Ljava/lang/String;I)Ljava/lang/String;");
		if (mid != nullptr)
		{
			jstring java_str = charTojstring(env, "�忾");
			jstring return_str = (jstring)env->CallObjectMethod(obj_1, mid, java_str, 10);
			const char* unicode_str = env->GetStringUTFChars(return_str, nullptr);
			if (unicode_str != nullptr)
			{
				// ��ӡ��ʹ��
				std::cout << unicode_str << std::endl;
				// �ͷ� Unicode �ַ���
				env->ReleaseStringUTFChars(return_str, unicode_str);
			}
			else {
				u_out(env, cls, "�ַ���ת��ʧ��!-append");
			}
		}
		else {
			u_out(env, cls, "����ID��ȡʧ��!-append");
		}
	}
	else {
		u_out(env, cls, "�����ʧ��!-append");
	}
	//��������:
	//�쳣����
	//jobject obj_2 = env->AllocObject(cls);//��ȡĬ�Ϲ����ʵ��
	jobject obj_2 = demo;//������ʹ��������һ�������½��Ĺ����ʵ��
	if (cls != nullptr)
	{
		mid = env->GetMethodID(cls, "throwExcp", "()V");
		if (mid != nullptr)
		{
			env->CallVoidMethod(obj_2, mid);
			//��ȡ�쳣��Ϣ
			jthrowable excp = 0;
			excp = env->ExceptionOccurred();
			if (excp)
			{
				jclass cls = env->GetObjectClass(excp);
				jmethodID mid = env->GetMethodID(cls, "toString", "()Ljava/lang/String;");
				jstring return_str = (jstring)env->CallObjectMethod(excp, mid);
				//ת���ַ��������������
				const char* unicode_str = env->GetStringUTFChars(return_str, nullptr);
				if (unicode_str != nullptr)
				{
					// ��ӡ��ʹ��
					std::cout << unicode_str << std::endl;
					// �ͷ� Unicode �ַ���
					env->ReleaseStringUTFChars(return_str, unicode_str);
				}
				else
				{
					std::cout << "�ַ���ת��ʧ��!-throwExcp" << std::endl;
				}
				//����쳣״̬
				env->ExceptionClear();
			}
		}
		else {
			u_out(env, cls, "����ID��ȡʧ��!-throwExcp");
		}
	}
	else {
		u_out(env, cls, "�����ʧ��!-throwExcp");
	}
	//������:��
	//�������Ĺ���ͷ���
	int int_array[] = { 1,2,3,4,5,6 };//������jint����߼�����
	//�������Ԫ�ظ����Ĺ�ʽ�����ܴ�С��һ��Ԫ�ش�С��long��long long��������
	jintArray array = env->NewIntArray(sizeof(int_array) / sizeof(*int_array));
	//������������￪ʼ��䣬���������(�����Ǵ�0Ҳ���Ǵ�ͷ��ʼ��һֱ�������������)
	env->SetIntArrayRegion(array, 0, sizeof(int_array) / sizeof(*int_array), int_array);
	//��ʼ����
	jobject obj_3 = demo;
	mid = env->GetMethodID(cls, "setCounts", "([I)V");
	if (mid != nullptr)
	{
		env->CallVoidMethod(obj_3, mid, array);
	}
	//��ȡ���������
	mid = env->GetMethodID(cls, "getCounts", "()[I");
	if (mid != nullptr)
	{
		jintArray msg = (jintArray)env->CallObjectMethod(obj_3, mid, array);
		int len = env->GetArrayLength(msg);
		jint* elems = env->GetIntArrayElements(msg, 0);
		for (int i = 0; i < len; i++)
		{
			//cout << "�±�: " << i << " ֵΪ: " << elems[i] << endl;
			u_out(env, cls, "�±�: "), std::cout << i, u_out(env, cls, " ֵΪ: "), std::cout << elems[i] << std::endl;
		}
		env->ReleaseIntArrayElements(msg, elems, 0);
	}
	/*************************************�ƺ���**************************************/

	//������������ͷŶ�̬��
	jvm->DestroyJavaVM();
	::FreeLibrary(hInstance);
	return 0;
}

//utf8/16ת����gb2312(����ǵ�free�ͷ��ڴ�!������Ҫʹ�øú�������Ȼ����free�ᵼ���ڴ�й¶)
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

//gb2312ת����utf8/16
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
		// �ͷ��ڴ�
		free(buffer);
	}
	return rtn;
}

void u_out(JNIEnv* env, jclass java_class, const char* str)
{
	//����demoת��
	//jobject obj = env->AllocObject(java_class);//��ȡĬ�Ϲ����ʵ��
	//jmethodID mid;
	//if (java_class != nullptr)
	//{
	//	//�������Ķ��ǩ���ǲ���Ҫ;�����ģ������;ֻ��Stringǩ���Դ��ģ����������int�β��Ǿ���(III)Ljava/lang/String;
	//	mid = env->GetMethodID(java_class, "setStrData", "(Ljava/lang/String;)Ljava/lang/String;");
	//	if (mid != nullptr)
	//	{
	//		jstring java_str = charTojstring(env, str);
	//		jstring return_str = (jstring)env->CallObjectMethod(obj, mid, java_str);
	//		const char* unicode_str = env->GetStringUTFChars(return_str, nullptr);
	//		if (unicode_str != nullptr)
	//		{
	//			// ��ӡ��ʹ��
	//			std::cout << unicode_str;
	//			// �ͷ� Unicode �ַ���
	//			env->ReleaseStringUTFChars(return_str, unicode_str);
	//			return;
	//		}
	//		
	//	}
	//	
	//}

	//ֱ��ת��
	jstring java_str = charTojstring(env, str);
	const char* unicode_str = env->GetStringUTFChars(java_str, nullptr);
	if (unicode_str != nullptr)
	{
		// ��ӡ��ʹ��
		std::cout << unicode_str;
		// �ͷ� Unicode �ַ���
		env->ReleaseStringUTFChars(java_str, unicode_str);
		return;
	}

	std::cout << "ERROR_2" << std::endl;
}