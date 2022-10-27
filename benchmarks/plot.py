import os
import argparse
import pandas as pd
import json
import jinja2
import textwrap
import altair as alt

# Add the plots as separate charts - so they can be downloaded individually
HTML_TEMPLATE = r"""
<!DOCTYPE html>
<html>
<head>
  <script src="https://cdn.jsdelivr.net/npm/vega@{{vega_version}}"></script>
  <script src="https://cdn.jsdelivr.net/npm/vega-lite@{{vegalite_version}}"></script>
  <script src="https://cdn.jsdelivr.net/npm/vega-embed@{{vegaembed_version}}"></script>
</head>
<body>
{% for _ in charts -%}
<div id="vis{{loop.index}}"></div>
{% endfor -%}
<script type="text/javascript">
{% for chart in charts -%}
vegaEmbed('#vis{{loop.index}}', {{chart}}).catch(console.error);
{% endfor -%}
</script>
</body>
</html>
""".strip()


def bps_to_mbps(bps: float):
    return bps / (1024 * 1024)


def clean_dataframe(df: pd.DataFrame):
    agg_list = df["aggregate_name"]
    checks = len(agg_list)
    dropped_indices = []
    for i in range(checks):
        if type(df["aggregate_name"][i]) == str:
            dropped_indices.append(i)
    df.drop(dropped_indices, inplace=True)
    df.drop(columns="aggregate_name", inplace=True)
    df.reset_index(inplace=True)


def plot_throughput(df: pd.DataFrame):
    """
    Plots the speed of the different stacks for the region arithmetic
    operations.
    :param df: The dataframe containing the results
    """

    benchmark_names = []

    for i in range(len(df["name"])):
        name_indices = df["name"][i].split("/")
        benchmark_name = name_indices[0].replace("BM_", "")
        benchmark_names.append(benchmark_name)
    df = df.loc[
        :,
        [
            "name",
            "repetitions",
            "repetition_index",
            "bytes_per_second",
            "size",
        ],
    ]

    df.insert(0, "benchmark", benchmark_names)

    df.rename(columns={"repetitions": "runs"}, inplace=True)
    df.rename(columns={"repetition_index": "run_index"}, inplace=True)
    df.rename(columns={"bytes_per_second": "throughput"}, inplace=True)

    df["run_index"] = df["run_index"].astype(int)
    df["size"] = df["size"].astype(int)
    df["throughput"] = df["throughput"].apply(bps_to_mbps)

    fixed_fields = ["benchmark"]

    op = df.groupby(fixed_fields)

    charts = []

    for key, group in op:

        title = key.replace("_", " ").capitalize()

        config = textwrap.wrap(
            ", ".join(
                ["{} = {}".format(f, k) for f, k in zip(fixed_fields[1:], key[1:])]
            )
        )

        chart = (
            alt.Chart(group)
            .mark_point()
            .encode(
                x=alt.X("run_index:Q", axis=alt.Axis(title="Runs [-]")),
                y=alt.Y("throughput:Q", axis=alt.Axis(title="Throughput [MB/s]")),
                color=alt.Color("name:N", legend=alt.Legend(title="Configuration")),
            )
            .properties(title={"text": title, "subtitle": config})
        )

        chart += (
            alt.Chart(group).mark_rule().encode(y="mean(throughput):Q", color="name:N")
        )
        charts.append(chart.to_json(indent=None))

    chart_path = os.path.abspath(os.path.join(".", "benchmark_results/throughput.html"))

    with open(chart_path, "w") as f:
        template = jinja2.Template(HTML_TEMPLATE)
        output = template.render(
            vega_version=alt.VEGA_VERSION,
            vegalite_version=alt.VEGALITE_VERSION,
            vegaembed_version=alt.VEGAEMBED_VERSION,
            charts=charts,
        )

        f.write(output)

    print("Writing block_chart.html...")


if __name__ == "__main__":

    print("Using pandas {}".format(pd.__version__))

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-i",
        action="store",
        dest="input",
        required=True,
        help="The input .json file produced by the " "kodo block benchmark",
    )

    args = parser.parse_args()

    with open(args.input, "r") as file:
        data = json.load(file)

    results = data["benchmarks"]

    df = pd.DataFrame(results)

    clean_dataframe(df)

    plot_throughput(df)

    print("Plotting operations")
