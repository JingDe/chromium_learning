#ifndef SANDBOX_LINUX_SUID_CLIENT_SETUID_SANDBOX_CLIENT_H_
#define SANDBOX_LINUX_SUID_CLIENT_SETUID_SANDBOX_CLIENT_H_

#include<memory>

#include"sandbox/sandbox_export.h"

namespace sanbox {


// ʹ��setuid sandbox�ĸ����ࡣ�ڱ�setuid helper��sandbox.cc��ִ�к�ʹ��.
// ����A����ɳ�仯�Ľ���B�ĵ����÷���
// 
// 5,Bʹ��CloseDummyFile()�رտ��ļ�������
// 6,Bִ�г�ʼ���������ļ�ϵͳ
// 6, (��ѡ) Bʹ��sandbox::Credentials::HasOpenDirectory()��֤û�д򿪵�Ŀ¼
// 7,B׼������init���̡�B���ܽ����κ��������̵���Ϣ������SIGKILL��
// 
// 8,B����ChrootMe()����chroot��ͨ��״̬������������ɳ��״̬
class SANDBOX_EXPORT SetuidSandboxClient {

};

}

#endif