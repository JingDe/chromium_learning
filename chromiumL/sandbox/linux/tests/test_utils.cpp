#include"sandbox/tests/test_utils.h"

#include<sys/wait.h>

#include"base/posix/eintr_wrapper.h"
#include"base/logging.h"

/*
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
�ȴ�һ���ӽ��̸ı�״̬
����ǰ�̣߳�ֱ�����߳����ڽ��̵�ĳ���ӽ��̸ı�״̬
����ڵ���waitid֮ǰ��һ���ӽ��̸ı���״̬��waitid��������
infop����һ���ӽ��̵ĵ�ǰ״̬;idtype��idָ���ȴ����ӽ��̣�
���ָ��NOHANG����û���ӽ��̵ȴ�������0����������ӽ���״̬�ı䷵�أ�����ֵΪ0������-1
*/
bool TestUtils::CurrentProcessHasChildren()
{
	siginfo_t process_info;
	int ret = HANDLE_EINTR(waitid(P_ALL, 0, &process_info, WEXITED | WNOHANG | WNOWAIT));
	if (-1 == ret)
	{
		PCHECK(ECHILD == errno); // ECHILD���ý���û�������˳���û��wait���ӽ���
		return false; // ??û���ӽ���
	}
	else // ??���ӽ���
		return true;
}
