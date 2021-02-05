import os
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas
import math


font = {'size'   : 18}
mpl.rc('font', **font)
mpl.rcParams['axes.spines.right'] = False
mpl.rcParams['axes.spines.top'] = False
mpl.rcParams['axes.titlesize'] = font["size"]-2


def wasmatiVwassail():
    comparisonFilePath = os.path.join(os.path.dirname(__file__), "polybench-wasmati-v-wassail.csv")
    data = pandas.read_csv(comparisonFilePath)
    labels = data["name"].tolist()
    wasmati = data["Wasmati"].tolist()
    wassail = data["Wassail"].tolist()

    colors=["#175959", "#F5501F"]

    x = np.arange(len(labels))  # the label locations
    width = 0.35  # the width of the bars

    fig, ax = plt.subplots(figsize=(8,3))
    rects1 = ax.bar(x - width/2, wasmati, width, label='Wasmati', color="#4FC1E8")
    rects2 = ax.bar(x + width/2, wassail, width, label='Wassail', color="#ED5564")

    plt.legend(loc="lower center", bbox_to_anchor=(0.5, 0.9), ncol=len(labels), frameon=False)
    plt.xticks(x, labels, rotation=65, ha='right')
    plt.ylabel("Times (ms)")
    plt.tight_layout()
    plt.savefig("polybench-wasmati-wassail.pdf", bbox_inches='tight')
    # plt.show()


def wasmatiSpecGeneration():
    specFilePath = os.path.join(os.path.dirname(__file__), "wasmati-spec.csv")
    data = pandas.read_csv(specFilePath)
    dataC = data.loc[data["Language"] == "C"]
    dataCPP = data.loc[data["Language"] == "C++"]

    data = [dataC, dataCPP]
    titles = ["C Language", "C++ Language"]

    colors = ["#AC92EB", "#4FC1E8", "#FFCE54", "#ED5564", "#A0D568"]
    

    def clean(int_str):
        return float(int_str[:-1])

    fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(9,7), gridspec_kw = {'wspace':0, 'hspace':0}, sharey=True, constrained_layout=False)
    
    ax_pos = 0
    for i,ax in enumerate(axes.flat):
        data_loc = data[ax_pos]
        labels = data_loc["name"].tolist()
        N = len(labels)

        parsing = np.array(list(map(clean, data_loc["parsing (%)"].tolist())))
        ast = np.array(list(map(clean, data_loc["ast (%)"].tolist())))
        cfg = np.array(list(map(clean, data_loc["cfg (%)"].tolist())))
        cg = np.array(list(map(clean, data_loc["cg (%)"].tolist())))
        pdg = np.array(list(map(clean, data_loc["pdg (%)"].tolist())))

        ind = np.arange(N)    # the x locations for the groups
        width = 0.45       # the width of the bars: can also be len(x) sequence

        p1 = ax.bar(ind, parsing, width, color=colors[0])
        p2 = ax.bar(ind, ast, width, bottom=parsing, color=colors[1])
        p3 = ax.bar(ind, cfg, width, bottom=parsing+ast, color=colors[2])
        p4 = ax.bar(ind, cg, width, bottom=parsing+ast+cfg, color=colors[3])
        p5 = ax.bar(ind, pdg, width, bottom=parsing+ast+cfg+cg, color=colors[4])

        ax.set(ylabel="Times (%)")
        ax.label_outer()
        ax.set_title(titles[ax_pos], y=0.96)
        ax.set_xticks(ind)
        ax.set_xticklabels(labels, rotation=65, ha='right')

        ax_pos+=1

    legend_ps = (p1[0], p2[0], p3[0], p4[0], p5[0])
    legend_labels = ("Parsing", "AST", "CFG", "CG", "PDG")
    plt.legend(legend_ps, legend_labels, loc="lower center", bbox_to_anchor=(-0.1, 1), ncol=len(labels), frameon=False)
    plt.tight_layout()
    plt.savefig("generation-wasmati.pdf", bbox_inches='tight')
    # plt.show()


def wasmatiEdgeTypes():
    specFilePath = os.path.join(os.path.dirname(__file__), "wasmati-spec.csv")
    data = pandas.read_csv(specFilePath)
    dataC = data.loc[data["Language"] == "C"]
    dataCPP = data.loc[data["Language"] == "C++"]

    data = [dataC, dataCPP]
    titles = ["C Language", "C++ Language"]

    colors = ["#AC92EB", "#4FC1E8", "#FFCE54", "#ED5564", "#A0D568"]
    

    def clean(int_str):
        return float(int_str[:-1])

    fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(9,7), gridspec_kw = {'wspace':0, 'hspace':0}, sharey=True, constrained_layout=False)
    
    ax_pos = 0
    for i,ax in enumerate(axes.flat):
        data_loc = data[ax_pos]
        labels = data_loc["name"].tolist()
        N = len(labels)

        ast = np.array(list(map(clean, data_loc["ast2 (%)"].tolist())))
        cfg = np.array(list(map(clean, data_loc["cfg2 (%)"].tolist())))
        cg = np.array(list(map(clean, data_loc["cg2 (%)"].tolist())))
        pdg = np.array(list(map(clean, data_loc["pdg2 (%)"].tolist())))

        ind = np.arange(N)    # the x locations for the groups
        width = 0.45       # the width of the bars: can also be len(x) sequence

        p1 = ax.bar(ind, ast, width, color=colors[1])
        p2 = ax.bar(ind, cfg, width, bottom=ast, color=colors[2])
        p3 = ax.bar(ind, cg, width, bottom=ast+cfg, color=colors[3])
        p4 = ax.bar(ind, pdg, width, bottom=ast+cfg+cg, color=colors[4])

        ax.set(ylabel="Edges (%)")
        ax.label_outer()
        ax.set_title(titles[ax_pos], y=0.96)
        ax.set_xticks(ind)
        ax.set_xticklabels(labels, rotation=65, ha='right')

        ax_pos+=1

    legend_ps = (p1[0], p2[0], p3[0], p4[0])
    legend_labels = ("AST", "CFG", "CG", "PDG")
    plt.legend(legend_ps, legend_labels, loc="lower center", bbox_to_anchor=(0, 1), ncol=len(labels), frameon=False)
    plt.tight_layout()
    plt.savefig("wasmati-edges.pdf", bbox_inches='tight')
    #plt.show()


def scatterWasmati():
    masterFilePath = os.path.join(os.path.dirname(__file__), "wasmati-master.csv")
    data = pandas.read_csv(masterFilePath)[["Nodes + Edges", "time (s)"]]

    data = data.sort_values(by="Nodes + Edges")

    def power_law(x, a, b):
        return a*np.power(x, b)

    x = np.asarray(data["Nodes + Edges"], dtype=float)
    y = np.asarray(data["time (s)"], dtype=float)

    def power_func(xs):
        # 2.53e-05*​x^​0.902
        return 2.5e-5 * x**0.902 


    plt.scatter(x, y, label='Time', color="#4FC1E8")
    plt.plot(x, power_func(x), label="Trend line", linestyle='--', linewidth=2, color='#ED5564')

    plt.yscale("log")
    plt.ylabel("Time (s)")

    plt.xscale("log")
    plt.xlabel("Number of Nodes + Edges")
    
    plt.legend(frameon=False)
    plt.tight_layout()
    plt.savefig("wasmati-construction-time.pdf", bbox_inches='tight')
    # plt.show()


def queryExecution():
    execFilePath = os.path.join(os.path.dirname(__file__), "wasmati-exec-agg.csv")
    data = pandas.read_csv(execFilePath)
    data.replace("TIMEOUT", 600000, inplace=True)

    labels = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"]

    native = []
    wql = []
    neo4j = []
    datalog = []
    for i in labels:
        native.append(math.ceil(data[data["tool"] == "Nativo"][i].astype(int).mean()))
        wql.append(math.ceil(data[data["tool"] == "Wasmati"][i].astype(int).mean()))
        neo4j.append(math.ceil(data[data["tool"] == "Neo4J"][i].astype(int).mean()))
        datalog.append(math.ceil(data[data["tool"] == "Datalog"][i].astype(int).mean()))

    x = np.arange(len(labels))  # the label locations
    width = 0.2  # the width of the bars

    colors = ["#4FC1E8", "#FFCE54", "#ED5564", "#A0D568"]

    fig, ax = plt.subplots(figsize=(12, 6))
    rects1 = ax.bar(x - width*1.5, native, width, label='Native', color=colors[0])
    rects2 = ax.bar(x - width/2, wql, width, label='Wasmati', color=colors[1])
    rects3 = ax.bar(x + width/2, neo4j, width, label='Neo4J', color=colors[2])
    rects4 = ax.bar(x + width*1.5, datalog, width, label='Datalog', color=colors[3])

    plt.legend(loc="lower left", bbox_to_anchor=(0.06, 1), ncol=len(labels), frameon=False)    
    plt.xticks(x, labels)
    plt.yscale("log")
    plt.ylabel("Times (ms)")

    plt.tight_layout()

    plt.savefig("wasmati-exec-mean.pdf", bbox_inches='tight')
    #plt.show()


def queryImportExecution():
    execFilePath = os.path.join(os.path.dirname(__file__), "wasmati-exec-agg.csv")
    data = pandas.read_csv(execFilePath)
    data.replace("TIMEOUT", 600000, inplace=True)

    labels = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"]

    native_import = math.ceil(data[data["tool"] == "Nativo"]["import"].astype(int).mean())

    wql_import = math.ceil(data[data["tool"] == "Wasmati"]["import"].astype(int).mean()) 

    neo4j_import = math.ceil(data[data["tool"] == "Neo4J"]["import"].astype(int).mean())
    neo4j_unzip = math.ceil(data[data["tool"] == "Neo4J"]["unzip"].astype(int).mean())

    datalog_import = math.ceil(data[data["tool"] == "Datalog"]["import"].astype(int).mean())
    datalog_unzip = math.ceil(data[data["tool"] == "Datalog"]["unzip"].astype(int).mean())

    native = []
    wql = []
    neo4j = []
    datalog = []
    for i in labels:
        native_mean = math.ceil(data[data["tool"] == "Nativo"][i].astype(int).mean())
        wql_mean = math.ceil(data[data["tool"] == "Wasmati"][i].astype(int).mean())
        neo4j_mean = math.ceil(data[data["tool"] == "Neo4J"][i].astype(int).mean())
        datalog_mean = math.ceil(data[data["tool"] == "Datalog"][i].astype(int).mean())

        native.append(native_mean + native_import)
        wql.append(wql_mean + wql_import)
        neo4j.append(neo4j_mean + neo4j_import + neo4j_unzip)
        datalog.append(datalog_mean + datalog_import + datalog_unzip)

    x = np.arange(len(labels))  # the label locations
    width = 0.2  # the width of the bars

    colors = ["#4FC1E8", "#FFCE54", "#ED5564", "#A0D568"]

    fig, ax = plt.subplots(figsize=(12, 6))
    rects1 = ax.bar(x - width*1.5, native, width, label='Native', color=colors[0])
    rects2 = ax.bar(x - width/2, wql, width, label='Wasmati', color=colors[1])
    rects3 = ax.bar(x + width/2, neo4j, width, label='Neo4J', color=colors[2])
    rects4 = ax.bar(x + width*1.5, datalog, width, label='Datalog', color=colors[3])

    plt.legend(loc="lower left", bbox_to_anchor=(0.1, 1), ncol=len(labels), frameon=False)    
    plt.xticks(x, labels)
    # plt.ylim(20000, 350000)
    plt.yscale("log", subs=[])
    plt.yticks([25000, 50000, 100000, 200000, 300000], ["25", "50", "100", "200", "300"])
    plt.ylabel("Times (s)")
    plt.xlabel("Queries")

    plt.tight_layout()

    plt.savefig("wasmati-exec-import-mean.pdf", bbox_inches='tight')
    #plt.show()


def main():
    #wasmatiVwassail()
    #wasmatiSpecGeneration()
    #wasmatiEdgeTypes()
    #scatterWasmati()
    #queryExecution()
    queryImportExecution()

main()
