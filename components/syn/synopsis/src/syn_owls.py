import dash
from dash import dcc, html
import plotly.express as px
import pandas as pd
from dash.dependencies import Input, Output
import os

total_size_sc = 0.0
total_size_dl = 0.0

app = dash.Dash(__name__)
app.layout = html.Div(
    html.Div([
        html.H4('SYNOPSIS Data Display'),
        dcc.Graph(id='live-update-graph-sc'),
        html.Div(id='live-update-size-sc'),
        dcc.Graph(id='live-update-graph-dl'),
        html.Div(id='live-update-size-dl'),
        dcc.Interval(
            id='interval-component',
            interval=1*1000,
            n_intervals=0
        )
    ])
)

@app.callback(Output('live-update-graph-sc', 'figure'),
              Input('interval-component', 'n_intervals'))
def update_graph_sc(n):
    path = "/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/spacecraft"
    files_list = os.listdir(path)
    rounded_file_sizes = []
    total_size = 0.0

    for file in files_list:
        name = os.path.join(path,file)
        size = os.stat(name).st_size
        rounded_file_sizes.append(round(size/(1024*1024),3))
        total_size += size

    df = pd.DataFrame({
        "SpaceCraft Files": files_list,
        "Size(MB)": rounded_file_sizes
    })

    fig = px.bar(df, x="SpaceCraft Files", y="Size(MB)", barmode="group")
   
    global total_size_sc
    total_size_sc = round(total_size/(1024*1024), 3)

    return fig

@app.callback(Output('live-update-size-sc', 'children'),
              Input('interval-component', 'n_intervals'))
def update_size_sc(n):
    global total_size_sc
    style = {'padding': '5px', 'fontSize': '16px'}
    return [
        html.Span('SC Storage in use: {0:.3f} MB'.format(total_size_sc), style=style)
    ]
    
                     
@app.callback(Output('live-update-graph-dl', 'figure'),
              Input('interval-component', 'n_intervals'))
def update_graph_dl(n):
    path = "/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/downlink"
    files_list = os.listdir(path)
    rounded_file_sizes = []
    total_size = 0.0

    for file in files_list:
        name = os.path.join(path,file)
        size = os.stat(name).st_size
        rounded_file_sizes.append(round(size/(1024*1024),3))
        total_size += size

    df = pd.DataFrame({
        "Downlink Files": files_list,
        "Size(MB)": rounded_file_sizes
    })

    fig = px.bar(df, x="Downlink Files", y="Size(MB)", barmode="group")

    global total_size_dl
    total_size_dl = round(total_size/(1024*1024),3)

    return fig

@app.callback(Output('live-update-size-dl', 'children'),
              Input('interval-component', 'n_intervals'))
def update_size_sc(n):
    global total_size_dl
    style = {'padding': '5px', 'fontSize': '16px'}
    return [
        html.Span('Total Downlinked: {0:.3f} MB'.format(total_size_dl), style=style)
    ]

if __name__ == "__main__":
    app.run_server(debug=True)
