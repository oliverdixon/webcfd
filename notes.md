# Theory Notes

## Definitions

For an array of $M$ microphones, the signal $x_m(t)$ captured at the $m$-th
microphone can be modelled as

$$
x_m(t) = \alpha_m s\left( t - t_m^{\vec{q}} \right) + u_m(t)
$$

where:

 * $s(t)$ is the source signal;
 * $u_m(t)$ is the combination of reverberation, interferences, and background
   noise;
 * $\alpha_m$ is the propagation attenuation;
 * $t_m^{\vec{q}}$ is the delay of the signal $s(t)$ from a source location
   $\vec{q}$ to the $m$-th microphone.

Equivalently, this can be represented in the frequency domain as

$$
X_m(\omega) = \alpha_m S(\omega) \exp\left( -i\omega\tau_m^{\vec{q}} \right) +
U_m(\omega)
$$

where

 * $\omega = 2\pi f T$ is the normalised frequency in radians-per-sample
   corresponding to the frequency $f \text{Hz}$ of the continuous-time signal
   $x_m(t)$;
 * $T$ is the sampling period (in seconds) of the continuous-time signal
   $x_m(t)$, such that $x_m[n] = x_m(nT)$;
 * $\tau_m^{\vec{q}} = t_m^{\vec{q}} / T$ is the normalised delay.

It is assumed that the signal is sampled above the Nyquist frequency, i.e.
$1/T > 2f_\text{max}$, where $f_\text{max}$ is the maximum frequency of the
signal.

## SSL Problem Formulation

We are attempting to locate, from the array of samples recorded from the $M$
microphones, the source location $\vec{q}$ of the signal. This is the Sound
Source Location (SSL) problem.

By taking Fourier transforms of the signals captured in the time domain, we
compute a vector of signals in the frequency domain:

$$
\left\\{ X_1(\omega), \ldots, X_M(\omega) \right\\}
$$

SSL may then be formulated as an optimisation problem such as maximum-likelihood
or maximum power of the filter-and-sum beamformer according to these frequency
counts. SRP-PHAT (*steered response power using the phase transform*) is one
technique to solving SSL with this setup.

## SRP-PHAT

First, the SRP method (steered response power) is an extension of generalised
cross-correlation (GCC) to multiple microphones. Broadly, SRP steers the
microphone arrayb to all possible candidate source locations to find the one
with the maximum power, typically using some frequency weighting. In SRP-PHAT,
PHAT refers to the phase transform choice of frequency weighting.

SRP-PHAT finds a source location by comparing the output powers of PHAT-weighted
filter-and-sum beamformers of different potential sound source locations in a
search region.

### Frequency Domain

In the frequency domain, the SRP-PHAT of a point $\vec{q}$ in space is defined
as

$$
P(\vec{q}) =
\sum_{m=1}^{M}
\sum_{l=1}^{M}
\int_{0}^{2\pi}
\frac{X_{m}(\omega) X_{l}^{\*}(\omega)}
{\left|X_{m}(\omega) X_{l}^{\*}(\omega)\right|}
\\,\exp(i \omega \tau_{ml}^{\vec{q}})
\\,d\omega .
$$

where $\tau_{ml}^{\vec{q}} = \tau_m^{\vec{q}} - \tau_l^{\vec{q}}$ is the
time-difference-on-arrival (TDOA) between microphones $m$ and $l$ at point
$\vec{q}$. $X^*_l(\omega)$ is the complex conjugate of $X_l(\omega)$.

We can reduce the number of computations by a factor of $M$ by rearranging:

$$
P(\vec{q}) =
\int_{0}^{2\pi}
\left|
\sum_{m=1}^{M}
\frac{X_{m}(\omega)}
{\left|X_{m}(\omega)\right|}
\\,\exp(i \omega \tau_{m}^{\vec{q}})
\right|^{2}
\\,d\omega
$$

We may then estimate the source location:

$$
\vec{q} \approx \mathop{\mathrm{arg\\,max}}_{\vec{q}\in\mathcal{Q}}\\, P\vec{q}
$$

where $\mathcal{Q}$ denotes the set of candidate locations.

### Time Domain

In the time domain, we can represent the SRP-PHAT as

$$
P(\vec{q}) = \sum_{m=1}^M \sum_{l=m+1}^M R_{ml}(\tau_{ml}^{\vec{q}})
$$

where $R_{ml}(\tau)$ is the PHAT-weighted GCCs between two microphones $m$ and
$l$:

$$
R_{ml}\left(\tau_{ml}^{\vec{q}}\right) =
\frac{1}{2\pi}
\int_{0}^{2\pi}
\frac{X_{m}(\omega) X_{l}^{\*}(\omega)}
{\left|X_{m}(\omega) X_{l}^{\*}(\omega)\right|}
\\,\exp(i \omega \tau)
\\,d\omega .
$$
