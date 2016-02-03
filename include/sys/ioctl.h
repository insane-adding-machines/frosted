#ifndef IOCTL_INCLUDE
#define IOCTL_INCLUDE

int ioctl(int fd, unsigned long request, void *arg);

/* GPIO */
#define IOCTL_GPIO_DISABLE 0
#define IOCTL_GPIO_ENABLE 1
#define IOCTL_GPIO_SET_OUTPUT 2
#define IOCTL_GPIO_SET_INPUT 3
#define IOCTL_GPIO_SET_PULLUPDOWN 4
#define IOCTL_GPIO_SET_ALT_FUNC 5

/* EXTI */
#define IOCTL_EXTI_DISABLE 0
#define IOCTL_EXTI_ENABLE 1

/* I2C */
#define IOCTL_I2C_SET_MASTER 0
#define IOCTL_I2C_SET_SLAVE  1
#define IOCTL_I2C_SET_ADDR7  2
#define IOCTL_I2C_SET_ADDR10 3
#define IOCTL_I2C_SET_ADDR7_2 4
#define IOCTL_I2C_SET_SPEED 5

/* NETWORK */
/* Routing table calls. */
#define SIOCADDRT	0x890B		/* add routing table entry	*/
#define SIOCDELRT	0x890C		/* delete routing table entry	*/
#define SIOCRTMSG	0x890D		/* call to routing system	*/

/* Socket configuration controls. */
#define SIOCGIFNAME	0x8910		/* get iface name		*/
#define SIOCSIFLINK	0x8911		/* set iface channel		*/
#define SIOCGIFCONF	0x8912		/* get iface list		*/
#define SIOCGIFFLAGS	0x8913		/* get flags			*/
#define SIOCSIFFLAGS	0x8914		/* set flags			*/
#define SIOCGIFADDR	0x8915		/* get PA address		*/
#define SIOCSIFADDR	0x8916		/* set PA address		*/
#define SIOCGIFDSTADDR	0x8917		/* get remote PA address	*/
#define SIOCSIFDSTADDR	0x8918		/* set remote PA address	*/
#define SIOCGIFBRDADDR	0x8919		/* get broadcast PA address	*/
#define SIOCSIFBRDADDR	0x891a		/* set broadcast PA address	*/
#define SIOCGIFNETMASK	0x891b		/* get network PA mask		*/
#define SIOCSIFNETMASK	0x891c		/* set network PA mask		*/
#define SIOCGIFMETRIC	0x891d		/* get metric			*/
#define SIOCSIFMETRIC	0x891e		/* set metric			*/
#define SIOCGIFMEM	0x891f		/* get memory address (BSD)	*/
#define SIOCSIFMEM	0x8920		/* set memory address (BSD)	*/
#define SIOCGIFMTU	0x8921		/* get MTU size			*/
#define SIOCSIFMTU	0x8922		/* set MTU size			*/
#define SIOCSIFNAME	0x8923		/* set interface name */
#define	SIOCSIFHWADDR	0x8924		/* set hardware address 	*/
#define SIOCGIFENCAP	0x8925		/* get/set encapsulations       */
#define SIOCSIFENCAP	0x8926		
#define SIOCGIFHWADDR	0x8927		/* Get hardware address		*/
#define SIOCGIFSLAVE	0x8929		/* Driver slaving support	*/
#define SIOCSIFSLAVE	0x8930
#define SIOCADDMULTI	0x8931		/* Multicast address lists	*/
#define SIOCDELMULTI	0x8932
#define SIOCGIFINDEX	0x8933		/* name -> if_index mapping	*/
#define SIOGIFINDEX	SIOCGIFINDEX	/* misprint compatibility :-)	*/
#define SIOCSIFPFLAGS	0x8934		/* set/get extended flags set	*/
#define SIOCGIFPFLAGS	0x8935
#define SIOCDIFADDR	0x8936		/* delete PA address		*/
#define	SIOCSIFHWBROADCAST	0x8937	/* set hardware broadcast addr	*/
#define SIOCGIFCOUNT	0x8938		/* get number of devices */

#define SIOCETHTOOL 0x8946      /* Ethtool interface        */

/* L3GD20 */
#define IOCTL_L3GD20_WRITE_CTRL_REG     0
#define IOCTL_L3GD20_READ_CTRL_REG      1

/* LSM303DLHC */
#define IOCTL_LSM303DLHC_WRITE_CTRL_REG     0
#define IOCTL_LSM303DLHC_READ_CTRL_REG      1

#endif
