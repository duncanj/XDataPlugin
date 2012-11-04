

#if BYTE_ORDER == LITTLE_ENDIAN
float custom_htonf(float x) {
	float r;
    //unsigned char *s1 = (unsigned char *) &x;
    //unsigned char *s2 = (unsigned char *) &r;
	char *s1 = (char *) &x;
	char *s2 = (char *) &r;
    s2[0] = s1[3];
    s2[1] = s1[2];
    s2[2] = s1[1];
    s2[3] = s1[0];
	return r;
}
long custom_htonl(long x) {
	long r;
    unsigned char *s1 = (unsigned char *) &x;
    unsigned char *s2 = (unsigned char *) &r;
    s2[0] = s1[3];
    s2[1] = s1[2];
    s2[2] = s1[1];
    s2[3] = s1[0];
	return r;
}
double custom_htond(double x) {
	double r;
    unsigned char *s1 = (unsigned char *) &x;
    unsigned char *s2 = (unsigned char *) &r;
    s2[0] = s1[7];
    s2[1] = s1[6];
    s2[2] = s1[5];
    s2[3] = s1[4];
    s2[4] = s1[3];
    s2[5] = s1[2];
    s2[6] = s1[1];
    s2[7] = s1[0];
	return r;
}

float custom_ntohf(float x) {
	float r;
    unsigned char *s1 = (unsigned char *) &x;
    unsigned char *s2 = (unsigned char *) &r;
    s2[0] = s1[3];
    s2[1] = s1[2];
    s2[2] = s1[1];
    s2[3] = s1[0];
	return r;
}
long custom_ntohl(long x) {
	long r;
    unsigned char *s1 = (unsigned char *) &x;
    unsigned char *s2 = (unsigned char *) &r;
    s2[0] = s1[3];
    s2[1] = s1[2];
    s2[2] = s1[1];
    s2[3] = s1[0];
	return r;
}
#else
#define custom_htonf(x) x;
#define custom_htonl(x) x;
#define custom_htond(x) x;
#define custom_ntohf(x) x;
#define custom_ntohl(x) x;
#endif

