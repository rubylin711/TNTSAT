
#ifndef __CTRL_INTF_H__
#define __CTRL_INTF_H__

enum wm_cmd_get_ids {
	WM_GET_SCAN_RESULTS,
	
};

int wm_cmd_get(int cmd, struct seq_file *s);
int wm_cmd_set(int argc, char *argv[]);

#endif	// __CTRL_INTF_H__
