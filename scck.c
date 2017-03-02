/*_____________________________________________
 |                                             |
 | SUN Cluster RG Check Version 1.0            |
 | --------------------------------------      |
 | Copyright (c) 2004 by Antoni Sawicki        |
 |_____________________________________________|
 |
 | Return 0 if a given resource group is running
 | on the local node, otherwise 1.
 |
 | Usage:
 |   scck <rg_name> && another command
 |
 | Or:
 | 
 |   if scck <rg_name>
 |   then
 |     command
 |   else
 |     another command
 |   fi
 |
 | Compile:
 |   gcc scck.c -o scck -I/usr/cluster/include -ldl  
 |
*/

#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <scha.h>
#include <dlfcn.h>

char *cprstr="SUN Cluster RG Check, v1.0, "\
             "Copyright (c) 2004 by Antoni Sawicki\n";

int main(int argc, char **argv) {

	// general
	int n, found=0, ret;
        char *rgname;

	// scha vars
        scha_err_t err;
	scha_rgstate_t	RGstate;
	scha_str_array_t *RGs;
	scha_cluster_t  hCL;
        scha_resourcegroup_t hRG;

	// ldl vars
	void *hSchaLib;
	int (*scha_resourcegroup_open)(void *, void *);
	int (*scha_resourcegroup_close)(void *);
	int (*scha_resourcegroup_get)(void *, void *, void *);
	int (*scha_cluster_open)(void *);
	int (*scha_cluster_close)(void *);
	int (*scha_cluster_get)(void *, void *, void *);


	//
	// bind scha functions
	//
	hSchaLib=dlopen("/usr/cluster/lib/libscha.so.1", RTLD_LAZY);
	if(hSchaLib==NULL) {
		fprintf(stderr, "Unable to load libscha.so.1\n");
		exit(1);
	}
	scha_resourcegroup_open=dlsym(hSchaLib, "scha_resourcegroup_open");
	scha_resourcegroup_close=dlsym(hSchaLib, "scha_resourcegroup_close");
	scha_resourcegroup_get=dlsym(hSchaLib, "scha_resourcegroup_get");
	scha_cluster_open=dlsym(hSchaLib, "scha_cluster_open");
	scha_cluster_close=dlsym(hSchaLib, "scha_cluster_close");
	scha_cluster_get=dlsym(hSchaLib, "scha_cluster_get");

	//
	// get list of resource groups
	//
	if(scha_cluster_open(&hCL) == SCHA_ERR_NOERR) {
		if(scha_cluster_get(hCL, SCHA_ALL_RESOURCEGROUPS, &RGs) != SCHA_ERR_NOERR) {
			scha_cluster_close(hCL);
			fprintf(stderr, "Unable to obtain list of RGs\n");
			exit(1);
		}
		scha_cluster_close(hCL);
	}

	//
	// usage check
	//
	rgname=argv[1];
	for(n=0; n<RGs->array_cnt; n++) 
		if(rgname && RGs->str_array[n] && strcmp(RGs->str_array[n], rgname)==0)
			found=1;

	if(argc<2 || !rgname || !found ) {
		fprintf(stderr, "Usage: %s <rg_name>\n"\
		"Will return 0 if <rg_name> is running on this node.\n",
		basename(argv[0]));

		fprintf(stderr, "Available resource groups: ");
		for(n=RGs->array_cnt; n>=0; n--)
			if(RGs->str_array[n])
				fprintf(stderr, "%s, ", RGs->str_array[n]);
		fprintf(stderr, "\n");
		exit(1);
	}
	

	//
	// see if specified resource group is online (on this machine)
	//
	ret=1;
        if(scha_resourcegroup_open(rgname, &hRG) == SCHA_ERR_NOERR) {
		if(scha_resourcegroup_get(hRG, SCHA_RG_STATE, &RGstate) == SCHA_ERR_NOERR) {
			scha_resourcegroup_close(hRG);

			if(RGstate==SCHA_RGSTATE_ONLINE) 
				ret=0;
			else
				ret=1;
		}
	}

	dlclose(hSchaLib);
	return ret;
}
