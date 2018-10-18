#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>

int path_exist(const char *name)
{
	struct  stat  buff; 
	if (lstat(name,&buff) < 0 )
		return 0;
	return 1;
}


int  is_dir (const char * name) 
{
	struct  stat  buff;  
	
	if (lstat(name,&buff) < 0 )
	  return -1; //if not exist name ,ignore
	
	/*if is directory return 1 ,else return 0*/ 
	if( S_ISDIR(buff.st_mode) )
		return 0;
	else 
		return -1;
}


int create_path(const char *path)
{
	char buf[PATH_MAX +1];
	int len;
	char *p ;

	len = strlen(path);
	if(len >= PATH_MAX )
		return -1;

	strcpy(buf, path);
	strcat(buf, "/");
	if(buf[0] != '/')
		return -1; 
	
 	p = buf;
	for(; *p == '/'; p++);
	if(*p == '\0')
		return 0;
	while(p)
	{
		p = strchr(p, '/');
		if(p)
		{
			*p = '\0';
			if(is_dir(buf) < 0)
				if(mkdir(buf, 0755) < 0)
					return -1;

			*p = '/';
			p++;
			for(; *p == '/'; p++); 
		}
	}

	return 0;
}
