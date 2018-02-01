# Introduction And Comparison On Random Graph Generation Algorithms

## Abstract

In network science and data analysis, randomly generated graphs are widely applied to evaluate the performance of existing algorithms and verify developed models. This article explains and compares a few typical random graph generation algorithms, discuss their different mathematical and/or practical properties, and then lists their advantages and disadvantages in practical usage.

### Keywords

Random graph, Stochastic process

## Introduction

In an era where quantity of information explodes, data analysis on graphs and networks is increasingly valued. A lot of data mining algorithms and models are developed to extract and exploit valuable information from existing graphs. To verify them, tons of data has to be collected, organized, and correctly labelled in order to construct a graph ready for analysis. However, such process involves with a large amount of manual work that is very painful and costly. In order to enable agile verification and iteration of new models, generation of heterogeneous random graphs is very important. Fortunately, many effective algorithms are already proposed and broadly adopted.

This article would list a couple of typical algorithms that model random undirected graphs, discuss the most fundamental ideas under the hood, and then compare them on different and practical perspectives. Afterward, some real-world graphs are provided in comparison to samples of our random graphs.

## Random Graphs

In mathematical points of view, a random graph is described by either probability distributions of vertices and edges or a stochastic process that generate it. Practically, two types of construction process are very common:

1. Create all nodes at once, and connect them upon a specific probability distribution
2. Add nodes by ones or batches, and wire(rewire) them on growth

Given some parameters determined, such as number of vertices and edges, a sample of the random graph could be created and put into use.

Before digging into concrete generation algorithms, some important indices for realistic graph evaluations are listed below:

1. **degree distribution**: probability distribution of degree of nodes in the graph
2. **average path length**: average smallest number of edges between any two nodes in the graph
3. **clustering coefficient**: an index that describe how nodes form communities

## Graph Generation Algorithms

### Erdős–Rényi model

Considering the first type of construction process, the Erdős–Rényi model further specify that the presence of edge between any two vertices is equally and independently probable, i.e. observes a uniform distribution. It may either be parameterized as $G(n, M)$ or $G(n, p)$, where $n$ is the number of vertices, $m$ is the number of edges and $p$ is the probability of a connection to form.

In $G(n,M)$ model, the total number of edges is deterministic, i.e. $M$. It is obvious that the ML estimation of connection rate is $p^{\prime}=\frac{M}{C^2_n}$.

Meanwhile, in $G(n,p)$ abstraction, the edge count $M^{\prime}$ is a random variable with a binomial probability distribution $P(M^{\prime})=p^{M}(1-p)^{C^2_n-M}, 0<M<C^2_n+1$ and an expected value of $p \times C^2_n$.

In this article, the latter model is preferred to be referred as ER model. By definition, ER model has a binomial degree distribution $P(k)=C^{k}_{n-1}p^k(1-p)^{n-1-k}$, where $k$ denotes the degree of a random vertex, and a fairly small clustering coefficient. The graph it samples is usually very dense and such topology is rare in reality.

![illustration of ER model]

### Barabási–Albert model

In real-world, many networks have the property of being scale-free; that is, their degree distribution follows a power law, instead of being binomial. Suppose $P(k)$ denotes the proportion nodes that have $k$ outgoing edges. The power law could be described as $P(k) - k^{-\gamma}$, where $\gamma$ typically falls in the range of $(2, 3)$.

The construction process of scale-free networks usually (but not necessarily) possess the following schemes:

1. Construct the whole graph by adding new nodes
2. Apply preferential attachment: vertices with more connections are more popular


Similar to the ER model, Barabási–Albert model requires a linear preferential attachment probability. For an existing node $n_i$, a new node would have a chance of $p_i=\frac{k_i}{\sum_j{k_j}}$ to be attached or connected, noting $k_i$ is degree of the particular vertex. Usually, the number of edges to be attached for each new node is denoted as a constant $m$. Therefore, the model may be denoted as $G(n,m)$.

For a model starts with a single isolated vertex, the edge count is also a deterministic constant $(n-1)\times m$. Different from previously mentioned ER model, graph samples of BA model usually forms a certain number of central nodes and possess a very recognizable topology structure. And more importantly, they are scale-free, where degree distribution would be around $P(k)-k^{-3}$ and average path length would be around $l - \frac{\ln{N}}{ln{ln{N}}}$.

To be noted, preferential attachment is not a requirement. And different random distribution for new attachment might work better in some circumstances.

![illustration of BA model]

### Watts–Strogatz model

On top of being scale-free, many real-world networks are also observed as following the analogy of small-world phenomenon, where length of path between any two nodes are relatively short in comparison to the scale of whole graph. Mathematically, a small-world network has a small average node distance and a significantly large clustering coefficient.

And the most trivial method for generating small-world graph is Watts-Strogatz model. This is also very simple an algorithm. It starts with a regular ring lattice graph with $N$ nodes connected to its $K$ neighbors. For each lattice edge from $n_i$ to $n_j$ where $i<j$, there is a chance of $\beta$ that a rewire process occur and a random new node $n_k$ is selected in place of $n_j$ in uniform distribution. In addition, the algorithm should always avoid some $n_k$ that may lead to self loop or duplicate edges. After all edges are visited, a small-world graph is done. 

Note properties such as degree distribution and average path length is rather complicated and rely heavily on the parameter $\beta$. Here, the formula would be skipped.

![illustration of WS model]

### Ensemble

According to the discussion above, although being carefully designed, both BA model and WS model cannot perfectly abstract our real-world needs. However, as graph construction process is incremental, we may first construct several sub-graphs as an autonomous system and then merge them into a single large network.

## Comparison



## Real-world Graphs

![illustration of Wiki Ref]