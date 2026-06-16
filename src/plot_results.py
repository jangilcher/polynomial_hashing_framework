# MIT License
#
# Copyright (c) 2023 Jan Gilcher, Jérôme Govinden
#               2025 Jan Gilcher, Jérôme Govinden
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from math import ceil
from os import makedirs
from pathlib import Path
from typing import Optional
import matplotlib.pyplot as plt
from cycler import cycler
import pandas as pd
from scipy.stats import zscore
from matplotlib.ticker import AutoMinorLocator

plot_colors: list[str] = cycler(linestyle=["-", "--", "-."]) * cycler(
    color=[
        "#1F77B4",
        "#FF7F0E",
        "#7F7F7F",
        "#17BECF",
        "#8C564B",
        "#D62728",
        "#FFBB78",
        "#2CA02C",
        "#9467BD",
        "#E377C2",
        "#BCBD22",
        "#AEC7E8",
        "#98DF8A",
        "#FF9896",
        "#C5B0D5",
        "#C49C94",
        "#F7B6D2",
        "#C7C7C7",
        "#DBDB8D",
        "#9EDAE5",
    ]
)


def replace_whitespace(s: str) -> str:
    return "_".join(s.split())


def plot(
    filename: str,
    name: str = "null",
    show_plots: bool = False,
    title: bool = True,
    latex: bool = False,
    maxsize: Optional[int] = None,
    fontsize: Optional[int] = None,
    y_cutoff: float = 5.0,
    keygen: bool = False,
    plot_dir: str = "plots/",
) -> None:
    if latex:
        params = {
            "text.usetex": True,
            "font.family": "Times",
        }
        if fontsize is not None:
            params["font.size"] = fontsize
        plt.rcParams.update(params)

    fname = replace_whitespace(name)
    png_dir: Path = plot_dir / "png"
    svg_dir: Path = plot_dir / "svg"
    png_dir.mkdir(parents=True, exist_ok=True)
    svg_dir.mkdir(parents=True, exist_ok=True)
    rawdata = pd.read_csv(filename, comment="#")
    zscores = rawdata.groupby(  # [["cycles", "MessageLength"]]
        "MessageLength", as_index=False, group_keys=False
    ).apply(
        zscore,
        # include_groups=False,
    )

    if keygen:
        rawdata["total"] = rawdata["cycles"] + rawdata["keygen"]
        rawdata["rate_total"] = rawdata["total"] / rawdata["MessageLength"]

    rawdata["rate"] = rawdata["cycles"] / rawdata["MessageLength"]
    data = (
        rawdata[abs(zscores["cycles"]) <= 3]
        .groupby("MessageLength", as_index=False)
        .mean()
    )
    std = (
        rawdata[abs(zscores["cycles"]) <= 3]
        .groupby("MessageLength", as_index=False)
        .std()
    )
    if maxsize is not None:
        data = data[data["MessageLength"] <= maxsize]
        std = std[std["MessageLength"] <= maxsize]

    data.plot(x="MessageLength", y="cycles", legend=False)
    plt.fill_between(
        data["MessageLength"],
        data["cycles"] - std["cycles"],
        data["cycles"] + std["cycles"],
        alpha=0.5,
    )
    plt.ylabel("Cycles")
    plt.xlabel("Message Length in Bytes")
    if title:
        plt.title(name)
    plt.grid(True)
    plt.grid(True, which="minor", axis="y", linestyle="--")

    ax = plt.gca()
    ax.set_ylim(bottom=0)
    ax.yaxis.set_minor_locator(AutoMinorLocator())
    ax.xaxis.set_minor_locator(AutoMinorLocator())

    plt.savefig(png_dir / f"{fname}_cycles.png", dpi=300, bbox_inches="tight")
    plt.savefig(svg_dir / f"{fname}_cycles.svg", dpi=300, bbox_inches="tight")

    data.plot(x="MessageLength", y="rate", legend=False)
    plt.fill_between(
        data["MessageLength"],
        data["rate"] - std["rate"],
        data["rate"] + std["rate"],
        alpha=0.5,
    )
    plt.ylabel("Cycles/byte")
    plt.xlabel("Message Length in Bytes")
    if title:
        plt.title(name)
    plt.grid(True)
    plt.grid(True, which="minor", axis="y", linestyle="--")

    ax = plt.gca()
    ax.set_ylim(bottom=0)
    ax.yaxis.set_minor_locator(AutoMinorLocator())
    ax.xaxis.set_minor_locator(AutoMinorLocator())
    plt.savefig(png_dir / f"{fname}_rate.png", dpi=300)
    plt.savefig(svg_dir / f"{fname}_rate.svg", dpi=300)

    ax.set_xlim(left=100)
    ax.set_ylim(
        top=ceil((data["rate"] + std["rate"])[data["MessageLength"] > 100].max())
    )
    plt.savefig(png_dir / f"{fname}_rate_cut.png", dpi=300, bbox_inches="tight")
    plt.savefig(svg_dir / f"{fname}_rate_cut.svg", dpi=300, bbox_inches="tight")

    ax.set_xlim(left=0)
    ax.set_ylim(top=y_cutoff)
    plt.savefig(png_dir / f"{fname}_rate_y_cut.png", dpi=300, bbox_inches="tight")
    plt.savefig(svg_dir / f"{fname}_rate_y_cut.svg", dpi=300, bbox_inches="tight")

    if keygen:
        data.plot(x="MessageLength", y="total", legend=False)
        plt.fill_between(
            data["MessageLength"],
            data["total"] - std["total"],
            data["total"] + std["total"],
            alpha=0.5,
        )
        plt.ylabel("Cycles")
        plt.xlabel("Message Length in Bytes")
        if title:
            plt.title(name)
        plt.grid(True)
        plt.grid(True, which="minor", axis="y", linestyle="--")

        ax = plt.gca()
        ax.set_ylim(bottom=0)
        ax.yaxis.set_minor_locator(AutoMinorLocator())
        ax.xaxis.set_minor_locator(AutoMinorLocator())

        plt.savefig(
            png_dir / f"{fname}_cycles_with_keygen.png", dpi=300, bbox_inches="tight"
        )
        plt.savefig(
            svg_dir / f"{fname}_cycles_with_keygen.svg", dpi=300, bbox_inches="tight"
        )

        data.plot(x="MessageLength", y="rate_total", legend=False)
        plt.fill_between(
            data["MessageLength"],
            data["rate_total"] - std["rate_total"],
            data["rate_total"] + std["rate_total"],
            alpha=0.5,
        )
        plt.ylabel("Cycles/byte")
        plt.xlabel("Message Length in Bytes")
        if title:
            plt.title(name)
        plt.grid(True)
        plt.grid(True, which="minor", axis="y", linestyle="--")

        ax = plt.gca()
        ax.set_ylim(bottom=0)
        ax.yaxis.set_minor_locator(AutoMinorLocator())
        ax.xaxis.set_minor_locator(AutoMinorLocator())
        plt.savefig(png_dir / f"{fname}_rate_with_keygen.png", dpi=300)
        plt.savefig(svg_dir / f"{fname}_rate_with_keygen.svg", dpi=300)

        ax.set_xlim(left=100)
        ax.set_ylim(
            top=ceil(
                (data["rate_total"] + std["rate_total"])[
                    data["MessageLength"] > 100
                ].max()
            )
        )
        plt.savefig(
            png_dir / f"{fname}_rate_cut_with_keygen.png", dpi=300, bbox_inches="tight"
        )
        plt.savefig(
            svg_dir / f"{fname}_rate_cut_with_keygen.svg", dpi=300, bbox_inches="tight"
        )

        ax.set_xlim(left=0)
        ax.set_ylim(top=y_cutoff)
        plt.savefig(
            png_dir / f"{fname}_rate_y_cut_with_keygen.png",
            dpi=300,
            bbox_inches="tight",
        )
        plt.savefig(
            svg_dir / f"{fname}_rate_y_cut_with_keygen.svg",
            dpi=300,
            bbox_inches="tight",
        )

    if show_plots:
        plt.show()

    plt.close("all")


def plot_compare(
    linenums: list[int],
    config: str = "config",
    labels: Optional[list[str]] = None,
    show_plots: bool = True,
    benchdir: str = "./",
    title: bool | str = True,
    latex: bool = False,
    maxsize: Optional[int] = None,
    fontsize: Optional[int] = None,
    y_cutoff: float = 5.0,
    plot_dir: str = "plots/",
) -> None:
    if latex:
        params = {
            "text.usetex": True,
            "font.family": "Times",
        }
        if fontsize is not None:
            params["font.size"] = fontsize
        plt.rcParams.update(params)

    png_dir: Path = plot_dir / "png"
    svg_dir: Path = plot_dir / "svg"
    png_dir.mkdir(parents=True, exist_ok=True)
    svg_dir.mkdir(parents=True, exist_ok=True)
    fig, ax = plt.subplots()
    ax.set_prop_cycle(plot_colors)
    if labels is None:
        labels = list(map(str, linenums))
    name = "_".join(list(map(str, linenums)))

    data_list = []
    std_list = []
    keygen = False
    for linenum in linenums:
        rawdata = pd.read_csv(f"{benchdir}{config}_{linenum}_results.csv", comment="#")
        if "keygen" in rawdata.columns:
            keygen |= True
            rawdata["total"] = rawdata["cycles"] + rawdata["keygen"]
            rawdata["rate_total"] = rawdata["total"] / rawdata["MessageLength"]
        else:
            rawdata["total"] = rawdata["cycles"]
            rawdata["rate_total"] = rawdata["total"] / rawdata["MessageLength"]

        rawdata["rate"] = rawdata["cycles"] / rawdata["MessageLength"]
        zscores = rawdata.groupby(
            "MessageLength", as_index=False, group_keys=False
        ).apply(
            zscore,
            # include_groups=False,
        )

        d = (
            rawdata[abs(zscores["cycles"]) <= 3]
            .groupby("MessageLength", as_index=False)
            .mean()
        )
        s = (
            rawdata[abs(zscores["cycles"]) <= 3]
            .groupby("MessageLength", as_index=False)
            .std()
        )
        if maxsize is not None:
            d = d[d["MessageLength"] <= maxsize]
            s = s[s["MessageLength"] <= maxsize]
        data_list.append(d)
        std_list.append(s)

    for data, std, label in zip(data_list, std_list, labels):
        ax = data.plot(ax=ax, x="MessageLength", y="cycles", label=label)
        ax.fill_between(
            data["MessageLength"],
            data["cycles"] - std["cycles"],
            data["cycles"] + std["cycles"],
            alpha=0.5,
        )
    ax.legend(
        loc="upper left",
        bbox_to_anchor=(-0.1, -0.225, 1.125, 0.102),
        mode="expand",
        ncol=2,
    )
    ax.set_ylabel("Cycles")
    ax.set_xlabel("Message Length in Bytes")
    if title:
        if isinstance(title, str):
            ax.set_title(title)
        else:
            ax.set_title(f"Cycles comparison of {config}")
    ax.set_ylim(bottom=0)
    ax.yaxis.set_minor_locator(AutoMinorLocator())
    ax.xaxis.set_minor_locator(AutoMinorLocator())
    # plt.tight_layout()
    plt.grid(True)
    plt.grid(True, which="minor", axis="y", linestyle="--")
    fig.savefig(
        png_dir / f"{config}_comparison_{name}_cycles.png", dpi=300, bbox_inches="tight"
    )
    fig.savefig(
        svg_dir / f"{config}_comparison_{name}_cycles.svg", dpi=300, bbox_inches="tight"
    )

    fig2, ax2 = plt.subplots()
    ax2.set_prop_cycle(plot_colors)
    maxima = []
    for data, std, label in zip(data_list, std_list, labels):
        maxima.append((data["rate"] + std["rate"])[data["MessageLength"] > 250].max())
        ax2 = data.plot(ax=ax2, x="MessageLength", y="rate", label=label)
        ax2.fill_between(
            data["MessageLength"],
            data["rate"] - std["rate"],
            data["rate"] + std["rate"],
            alpha=0.5,
        )
    ax2.legend(
        loc="upper left",
        bbox_to_anchor=(-0.1, -0.225, 1.125, 0.102),
        mode="expand",
        ncol=2,
    )
    ax2.set_ylabel("Cycles/byte")
    ax2.set_xlabel("Message Length in Bytes")
    if title:
        if isinstance(title, str):
            ax2.set_title(title)
        else:
            ax2.set_title(f"Rate comparison of {config}")
    ax2.set_ylim(bottom=0)
    ax2.xaxis.set_minor_locator(AutoMinorLocator())
    ax2.yaxis.set_minor_locator(AutoMinorLocator())
    # plt.tight_layout()
    plt.grid(True)
    plt.grid(True, which="minor", axis="y", linestyle="--")
    fig2.savefig(
        png_dir / f"{config}_comparison_{name}_rate.png", dpi=300, bbox_inches="tight"
    )
    fig2.savefig(
        svg_dir / f"{config}_comparison_{name}_rate.svg", dpi=300, bbox_inches="tight"
    )

    ax2.set_xlim(left=100)
    ax2.set_ylim(top=ceil(max(maxima)))
    fig2.savefig(
        png_dir / f"{config}_comparison_{name}_rate_cut.png",
        dpi=300,
        bbox_inches="tight",
    )
    fig2.savefig(
        svg_dir / f"{config}_comparison_{name}_rate_cut.svg",
        dpi=300,
        bbox_inches="tight",
    )

    ax2.set_xlim(left=0)
    ax2.set_ylim(top=y_cutoff)
    fig2.savefig(
        png_dir / f"{config}_comparison_{name}_rate_y_cut.png",
        dpi=300,
        bbox_inches="tight",
    )
    fig2.savefig(
        svg_dir / f"{config}_comparison_{name}_rate_y_cut.svg",
        dpi=300,
        bbox_inches="tight",
    )

    if keygen:
        fig, ax = plt.subplots()
        ax.set_prop_cycle(plot_colors)
        for data, std, label in zip(data_list, std_list, labels):
            ax = data.plot(ax=ax, x="MessageLength", y="total", label=label)
            ax.fill_between(
                data["MessageLength"],
                data["total"] - std["total"],
                data["total"] + std["total"],
                alpha=0.5,
            )
        ax.legend(
            loc="upper left",
            bbox_to_anchor=(-0.1, -0.225, 1.125, 0.102),
            mode="expand",
            ncol=2,
        )
        ax.set_ylabel("Cycles")
        ax.set_xlabel("Message Length in Bytes")
        if title:
            if isinstance(title, str):
                ax.set_title(title)
            else:
                ax.set_title(f"Cycles comparison of {config}")
        ax.set_ylim(bottom=0)
        ax.yaxis.set_minor_locator(AutoMinorLocator())
        ax.xaxis.set_minor_locator(AutoMinorLocator())
        # plt.tight_layout()
        plt.grid(True)
        plt.grid(True, which="minor", axis="y", linestyle="--")
        fig.savefig(
            png_dir / f"{config}_comparison_{name}_cycles_with_keygen.png",
            dpi=300,
            bbox_inches="tight",
        )
        fig.savefig(
            svg_dir / f"{config}_comparison_{name}_cycles_with_keygen.svg",
            dpi=300,
            bbox_inches="tight",
        )

        fig2, ax2 = plt.subplots()
        ax2.set_prop_cycle(plot_colors)
        maxima = []
        for data, std, label in zip(data_list, std_list, labels):
            maxima.append(
                (data["rate_total"] + std["rate_total"])[
                    data["MessageLength"] > 250
                ].max()
            )
            ax2 = data.plot(ax=ax2, x="MessageLength", y="rate_total", label=label)
            ax2.fill_between(
                data["MessageLength"],
                data["rate_total"] - std["rate_total"],
                data["rate_total"] + std["rate_total"],
                alpha=0.5,
            )
        ax2.legend(
            loc="upper left",
            bbox_to_anchor=(-0.1, -0.225, 1.125, 0.102),
            mode="expand",
            ncol=2,
        )
        ax2.set_ylabel("Cycles/byte")
        ax2.set_xlabel("Message Length in Bytes")
        if title:
            if isinstance(title, str):
                ax2.set_title(title)
            else:
                ax2.set_title(f"Rate comparison of {config}")
        ax2.set_ylim(bottom=0)
        ax2.xaxis.set_minor_locator(AutoMinorLocator())
        ax2.yaxis.set_minor_locator(AutoMinorLocator())
        # plt.tight_layout()
        plt.grid(True)
        plt.grid(True, which="minor", axis="y", linestyle="--")
        fig2.savefig(
            png_dir / f"{config}_comparison_{name}_rate_with_keygen.png",
            dpi=300,
            bbox_inches="tight",
        )
        fig2.savefig(
            svg_dir / f"{config}_comparison_{name}_rate_with_keygen.svg",
            dpi=300,
            bbox_inches="tight",
        )

        ax2.set_xlim(left=100)
        ax2.set_ylim(top=ceil(max(maxima)))
        fig2.savefig(
            png_dir / f"{config}_comparison_{name}_rate_cut_with_keygen.png",
            dpi=300,
            bbox_inches="tight",
        )
        fig2.savefig(
            svg_dir / f"{config}_comparison_{name}_rate_cut_with_keygen.svg",
            dpi=300,
            bbox_inches="tight",
        )

        ax2.set_xlim(left=0)
        ax2.set_ylim(top=y_cutoff)
        fig2.savefig(
            png_dir / f"{config}_comparison_{name}_rate_y_cut_with_keygen.png",
            dpi=300,
            bbox_inches="tight",
        )
        fig2.savefig(
            svg_dir / f"{config}_comparison_{name}_rate_y_cut_with_keygen.svg",
            dpi=300,
            bbox_inches="tight",
        )

    if show_plots:
        fig.show()
        fig2.show()

    plt.close("all")
