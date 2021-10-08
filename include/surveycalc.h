#ifndef _LLTOXY_H_

#ifdef _cplusplus
extern "C" {
#endif // _cplusplus

extern void ll2xy(double phi, double lambda, double phi0, double lambda0,
		double *x, double *y);
extern void xy2bl(double x, double y, double phi0, double lambda0, double phi1,
		double lambda1, double *phi, double *lambda);

#ifdef _cplusplus
}
#endif // _cplusplus
#endif // _LLTOXY_H_
