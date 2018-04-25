#include"process_util.h"

#include<cstdio>
#include<cinttypes>
#include<cstring>

#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>

static const int kMaxOomScore = 1000;
static const int kMaxOldOomScore = 15;

bool AdjustOOMScore(pid_t process, int score)
{
	if (score<0 || score>kMaxOomScore)
		return false;
	
	char oom_adj[27];
	// "/proc/" + log_10(2 * *64) + "\0"
	//    6     +       20     +     1         = 27
	snprintf(oom_adj, sizeof oom_adj, "/proc/%" PRIdMAX, (intmax_t)process); 
	// intmax_t最宽整数类型

	const int dirfd = open(oom_adj, O_RDONLY | O_DIRECTORY); // fcntl.h
	if (dirfd < 0)
		return false;

	struct stat statbuf;
	if (fstat(dirfd, &statbuf) < 0) // /proc/pid目录的信息
	{
		close(dirfd); // unistd.h
		return false;
	}
	
	if (getuid() != statbuf.st_uid) // 文件拥有者的用户ID
	{
		close(dirfd);
		return false;
	}

	int fd = openat(dirfd, "oom_score_adj", O_WRONLY);
	if (fd < 0)
	{
		fd = openat(dirfd, "oom_adj", O_WRONLY);
		if (fd < 0)
		{
			close(dirfd);
			return false;
		}
		else
			score = score * kMaxOldOomScore / kMaxOomScore;
	}
	close(dirfd);

	char buf[11];
	snprintf(buf, sizeof(buf), "%d", score);
	size_t len = strlen(buf);

	ssize_t bytes_written = write(fd, buf, len);
	close(fd);
	return (bytes_written == (ssize_t)len);
}