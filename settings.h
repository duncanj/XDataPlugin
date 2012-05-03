
// settings variables
extern int					dest_enable[];
extern char					dest_ip[][20];
extern unsigned short int	dest_port[];

extern unsigned short int   recv_port;
extern unsigned long int    recv_rate;
extern float                recv_delay;

// settings public functions
void	initSettings();
void	writeSettings();
