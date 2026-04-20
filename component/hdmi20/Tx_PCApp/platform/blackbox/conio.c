#include "conio.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int  _getch(void)
{
	struct termios tm, tm_old;
	int fd = 0, ch;

	if (tcgetattr(fd, &tm) < 0) {//�������ڵ��ն�����
		return -1;
	}

	tm_old = tm;
	cfmakeraw(&tm);//�����ն�����Ϊԭʼģʽ����ģʽ�����е������������ֽ�Ϊ��λ������
	if (tcsetattr(fd, TCSANOW, &tm) < 0) {//�����ϸ���֮�������
		return -1;
	}

	ch = getchar();
	if (tcsetattr(fd, TCSANOW, &tm_old) < 0) {//��������Ϊ���������
		return -1;
	}

	return ch;
}

int _kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	int ret;

	ret = tcgetattr(STDIN_FILENO, &oldt);
	if ( ret ) {
		return 0;
	}

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	ret = tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	if ( ret ) {
		return 0;
	}
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	ret = tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	if ( ret ) {
		return 0;
	}
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

void debug_key_wait(char *file, char *func, int line)
{
	char key;
	printf("\n%s_%s_%d, press 'g' to continue\n", file, func, line);
LOOP_START:
	while ( !_kbhit() ) { ; }
	key = (char)_getch();
	switch (key) {
		case 'g':
			//printf("\n%s_%s_%d\n",file,func,line);
			break;
		default:
			goto LOOP_START;
			break;
	}
}

void check_input_newline2delete(void)
{
	int c;
	while (1) {
		if (_kbhit()) {
			c = getchar();
			if ( c != 0xA && c != 0xD ) {
				ungetc(c, stdin);
			}
		} else {
			break;
		}
	}
}

