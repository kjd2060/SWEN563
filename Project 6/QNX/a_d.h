ifndef A_D_H
#define A_D_H

void request_io_access(void);
void map_io_ports(void);
void ad_Init(void);
double ad_convert(void);
int scale_converted_signal(double voltage_post_conversion);
void output_stm(int scaled_signal);

#endif // A_D_H
