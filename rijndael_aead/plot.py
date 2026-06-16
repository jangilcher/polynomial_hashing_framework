import matplotlib.pyplot as plt
from os import makedirs
import pandas as pd
from scipy.stats import zscore
from math import ceil, sqrt
from matplotlib import ticker
from matplotlib.ticker import AutoMinorLocator, LogLocator, FixedLocator, NullFormatter
from matplotlib.lines import Line2D
import matplotlib as mpl
import numpy as np

plot_colors: list[str] = [
    "#1F77B4",
    "#FF7F0E",
    "#2CA02C",
    "#D62728",
    "#9467BD",
    "#8C564B",
    "#E377C2",
    "#7F7F7F",
    "#BCBD22",
    "#17BECF",
    "#AEC7E8",
    "#FFBB78",
    "#98DF8A",
    "#FF9896",
    "#C5B0D5",
    "#C49C94",
    "#F7B6D2",
    "#C7C7C7",
    "#DBDB8D",
    "#9EDAE5",
]
styles  =[
    '-',
    '--',
    ':',
    '-.',
    (0, (2.5, 1)),
    (0, (3, 1, 1, 1)),
    (0, (3, 1, 1, 1, 1, 1, 1, 1)),
    (0, (5, 1)),
    (0, (1, 1)),
    (0, (7.5, 1)),
    (0, (3, 1, 3, 1, 1, 1)),
    (0, (3, 1, 3, 1, 1, 1, 1, 1)),
    (0, (3, 1, 3, 1, 1, 1, 1, 1, 1, 1)),
    ]

styles = ["-"]*len(plot_colors)

plotstep = 8 # only plot every 8th byte size, reduces filesize and prevents pgf/latex out of memory error. 
plotytop = 3.5


mpl.use('pgf')
mpl.rcParams.update({
    "pgf.texsystem": "pdflatex",
    "text.usetex": True,
    "font.family": "Times",
})
figx = 8 #8.5 * 2.54  # *4/3
figy = 6#*0.5 #11 * 2.54 # * 0.83  # *3/4
figy_mid = 6*0.575

def read_data(config):
    rawdata = pd.read_csv(f'bench/{config}_results.csv',comment="#")
    rawdata['rate'] = rawdata['cycles']/rawdata['MessageLength']
    data = rawdata.groupby('MessageLength', as_index=False).mean()
    data[["cycles_std", "rate_std"]] = rawdata.groupby('MessageLength', as_index=False).std()[["cycles", "rate"]]
    return data

def plot_compare(data_list, ax=None, colors=plot_colors, cycler=None, linestyle=None):
    if ax is None:
        fig,ax = plt.subplots(figsize=(figx, figy))
    if cycler is None:
        if colors is not None:
            if linestyle is None:
                linestyle = ['-']*len(colors)
            ax.set_prop_cycle(color=colors, linestyle=linestyle)
    else:
        ax.set_prop_cycle(cc)
    maxima = []
    for data in data_list:
        maxima.append((data['rate']+data['rate_std'])[data['MessageLength'] > 100].max())
        ax = data.plot(ax=ax, x='MessageLength', y='rate', legend=None, linewidth=.75)
        ax.fill_between(data['MessageLength'], data['rate']-data['rate_std'], data['rate']+data['rate_std'], alpha=0.5)
    return fig, ax

mhpnmh = "$1$-$\\mathsf{MHP}$-$1$-$\\mathsf{NMH}^{+}$"
poly = "\\mathsf{Poly}"
aes = "$\\mathsf{AES256}$"
rijndael = "$\\mathsf{Rijndael256}$"
chacha12 = "$\\mathsf{ChaCha12}$"
chacha16 = "$\\mathsf{ChaCha16}$"
chacha20 = "$\\mathsf{ChaCha20}$"

cfg_rijndael_aead_comp = [
    ("rijndael256x_poly128x4",      f"{rijndael}-${poly}128$", 0),
    ("rijndael256x_poly256x4",      f"{rijndael}-${poly}256$", 1),
    ("rijndael256x_mhp_nmh128x6",   f"{rijndael}-{mhpnmh}-$128$", 2),
    ("rijndael256x_mhp_nmh256x6",   f"{rijndael}-{mhpnmh}-$256$", 4),
], "rijndael_aead"

for config, name in [cfg_rijndael_aead_comp]:
    data = []
    for cfg,_,_ in config:
        d = read_data(cfg)
        data.append(d[d["MessageLength"] % plotstep == 0])
    color_list = [plot_colors[idx%len(plot_colors)] for _,_,idx in config]
    style_list = [styles[idx%len(styles)] for _,_,idx in config]
    f, ax = plot_compare(data,colors=color_list, linestyle=style_list)
    ax.xaxis.set_major_locator(ticker.MultipleLocator(1000*2))
    ax.xaxis.set_major_formatter(lambda x, pos: str(int(x/1000)))
    ax.set_xlim(left=0, right=16000)
    ax.set_ylim(top=plotytop)
    ax.grid(True)
    ax.grid(True, which='minor', axis='y', linestyle='--')
    ax.set_ylabel('Cycles/Byte')
    ax.set_xlabel('Message Length in kB')
    ax.set_ylim(bottom=0)
    ax.xaxis.set_minor_locator(AutoMinorLocator())
    ax.yaxis.set_minor_locator(AutoMinorLocator())
    legends_pf_128= [Line2D([0], [0], color=plot_colors[idx%len(plot_colors)], linestyle=styles[idx%len(styles)], lw=2, label=l.replace("_", "\_")) for (_, l, idx) in config]
    f.legend(handles=legends_pf_128, ncol=2, bbox_to_anchor=[0.5,0.0375], loc='upper center')
    f.savefig(f"{name}.png", dpi=300, bbox_inches='tight')
    f.savefig(f"{name}.svg", dpi=300, bbox_inches='tight')
    f.savefig(f"{name}.pgf", dpi=300, bbox_inches='tight')
    plt.close(f)

