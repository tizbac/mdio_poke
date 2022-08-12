#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
int new_ioctl_nums;
int skfd = -1;					/* AF_INET socket for ioctl() calls.	*/
struct ifreq ifr;


#if defined(SIOCGPARAMS)  && SIOCGPARAMS != SIOCDEVPRIVATE+3
#error Changed definition for SIOCGPARAMS
#else
#define SIOCGPARAMS (SIOCDEVPRIVATE+3) 		/* Read operational parameters. */
#define SIOCSPARAMS (SIOCDEVPRIVATE+4) 		/* Set operational parameters. */
#endif
int mdio_read(int skfd, int phy_id, int location)
{
	u16 *data = (u16 *)(&ifr.ifr_data);

	data[0] = phy_id;
	data[1] = location;

	if (ioctl(skfd, new_ioctl_nums ? 0x8948 : SIOCDEVPRIVATE+1, &ifr) < 0) {
		fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
		return -1;
	}
	return data[3];
}

void mdio_write(int skfd, int phy_id, int location, int value)
{
	u16 *data = (u16 *)(&ifr.ifr_data);

	data[0] = phy_id;
	data[1] = location;
	data[2] = value;

	if (ioctl(skfd, new_ioctl_nums ? 0x8949 : SIOCDEVPRIVATE+2, &ifr) < 0) {
		fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
	}
}

int main(int argc, char **argv) {
    
    if ( argc < 4 )
    {
        std::cerr << "Usage: mdio_poke \033[32;1miface\033[0m \033[33;1m0xregister\033[0m \033[33;1m0xvalue\033[0m" << std::endl;
        return 1;
    }
    
    if ((skfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
		perror("socket");
		return 1;
	}
	
	const char * ifname = argv[1];
	
	/* Verify that the interface supports the ioctl(), and if
	   it is using the new or old SIOCGMIIPHY value (grrr...).
	 */
	{
		u16 *data = (u16 *)(&ifr.ifr_data);

		strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
		data[0] = 0;

		if (ioctl(skfd, 0x8947, &ifr) >= 0) {
			new_ioctl_nums = 1;
		} else if (ioctl(skfd, SIOCDEVPRIVATE, &ifr) >= 0) {
			new_ioctl_nums = 0;
		} else {
			fprintf(stderr, "SIOCGMIIPHY on %s failed: %s\n", ifname,
					strerror(errno));
			(void) close(skfd);
			return 1;
		}
	}
	
	unsigned reg = std::stoul(argv[2], nullptr, 16);
    unsigned newval = std::stoul(argv[3], nullptr, 16);
	
	u16 *data = (u16 *)(&ifr.ifr_data);
	u32 *data32 = (u32 *)(&ifr.ifr_data);
    unsigned phy_id = data[0];
	mdio_write(skfd, phy_id, reg, newval);
    
    fprintf(stderr, "Set %04x to %04x\n" , reg, newval);
    
    
    newval = mdio_read(skfd, phy_id, reg);
    
    fprintf(stderr, "%04x reads back to %04x\n" , reg, newval);
    
    close(skfd);
    
    
    return 0;
}
