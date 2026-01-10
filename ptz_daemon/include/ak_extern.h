/* 'extern "C"' is the holy grail! */
#ifdef __cpp
extern "C"{
#endif

	enum ptz_turn_direction {
		PTZ_TURN_RESERVED = 0x00,
		PTZ_TURN_RIGHT,
		PTZ_TURN_LEFT,
		PTZ_TURN_DOWN,
		PTZ_TURN_UP
	};

    extern void ak_drv_ptz_open();
    extern int ak_drv_ptz_check_self(int);
    extern void ak_drv_ptz_set_degree(int,int);
    extern void ak_drv_ptz_setup_step_param(int,int,int,int);
    //extern void ak_drv_ptz_set_angle_rate(int,int);
    extern int ak_drv_ptz_turn(enum ptz_turn_direction, int);
    extern void ak_drv_ptz_turn_to_pos(int,int);
    extern int ak_drv_ptz_turn_stop(enum ptz_turn_direction);
    extern void ak_drv_ptz_set_speed(int,int, int, int);
    extern void ak_drv_ptz_get_version();
    extern void ak_drv_ptz_get_status(int,int);
    extern void ak_drv_ptz_close();
    extern void _TEST_ak_read_values();

    /* irled stuff */
    extern int ak_drv_ir_init(void);
    // 0xc long (memcpy)
    extern unsigned int ak_drv_ir_get_threshold(void *param_1,void *param_2);
    extern unsigned int ak_drv_ir_get_input_level
               (int, int, int, int);
    extern void ak_drv_ir_set_check_mode(int param_1);
    extern void ak_drv_ir_set_ircut(int param_1);
    extern void ak_drv_ir_set_threshold(void *param_1,void *param_2);



#ifdef __cpp
}
#endif
