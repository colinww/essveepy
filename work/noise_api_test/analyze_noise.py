###############################################################################
#
# UCSD ISPG Group 2022
#
# Created on 12-Nov-22
# @author: colinww
#
# Description
# -----------
# Load noise vectors for post-processing.
#
# Version History
# ---------------
# 12-Nov-22: Initial version
#
###############################################################################

import h5py
import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt

###########################################
# Load data and extract signals
###########################################
fp = h5py.File('sv_data_dump.h5')

rand_vec = np.array(fp['top']['random']['data'][:, 0])
randn_vec = np.array(fp['top']['random']['data'][:, 1])
randn_bnd_vec = np.array(fp['top']['random']['data'][:, 2])
flicker_vec = np.array(fp['top']['flicker']['data'])
fp.close()

###########################################
# Plot the uniform and normal distributions
###########################################

fh, axs = plt.subplots(3, 1, figsize=(12, 8), sharex=True,
                       constrained_layout=True)
axs[0].hist(rand_vec, bins=120, range=[-3, 3], density=True, facecolor='r',
            alpha=0.75)
axs[1].hist(randn_vec, bins=120, range=[-3, 3], density=True, facecolor='g',
            alpha=0.75)
axs[2].hist(randn_bnd_vec, bins=120, range=[-3, 3], density=True, facecolor='b',
            alpha=0.75)
for ax in axs:
    ax.grid(True)
    ax.set_ylabel('Density ()')
axs[-1].set_xlabel('Bin ()')
axs[-1].set_xlim([-3, 3])
axs[0].set_title('svpRandom::urand: uniform random variable on [0,1)')
axs[1].set_title('svpRandom::randn: normal random variable ($\sigma$=1)')
axs[2].set_title('svpRandom::randn_bnd: normal restricted to [-1.5, 1)')
fh.suptitle('Random number generators in svp_pkg')

########################
# Plot the flicker noise
########################
FS = 1e9
SPOT_AMP = 1e-18
SPOT_FREQ = FS / 1e3
NFFT = 131072

# Generate PSD
fv, pv = signal.welch(flicker_vec.flatten(), fs=FS, window='hann',
                      nperseg=NFFT, noverlap=NFFT // 3, detrend='constant',
                      scaling='density', return_onesided=False)
fh, ax = plt.subplots(1, 1, figsize=(12, 8), constrained_layout=True)
ax.loglog(fv[:NFFT // 2], pv[:NFFT // 2], lw=2)
ax.grid(True)
ax.set_xlim([fv[1], fv[NFFT // 2 -1]])
ax.set_ylabel('Density ($()^2$/Hz)')
ax.set_xlabel('Frequency (Hz)')
ax.set_title("Flicker noise sampled at {}GHz, ".format(FS / 1e9) +
             "spot density target of {} at {}MHz".format(
                 SPOT_AMP, SPOT_FREQ / 1e6))
fh.suptitle("Flicker noise generator svpFlicker")
