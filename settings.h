
// settings variables
extern int					dest_enable[];
extern char					dest_ip[][20];
extern unsigned short int	dest_port[];

extern unsigned short int   recv_port;
extern unsigned long int    recv_rate;
extern float                recv_delay;

extern unsigned long int    adc_data_rate;
extern unsigned long int    fms_data_rate;
extern unsigned long int    tcas_data_rate;
extern float                adc_data_delay;
extern float                avionics_data_delay;
extern float                engines_data_delay;
extern float                static_data_delay;
extern float                fms_data_delay;
extern float                tcas_data_delay;

// settings public functions
void	initSettings();
void	writeSettings();
