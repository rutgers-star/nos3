import dash
import subprocess
import json
import jinja2
import base64
import os
import socket

import dash_bootstrap_components    as dbc
import numpy                        as np
import plotly.express               as px
import pandas                       as pd

from dash.dependencies import Input, Output
from dash import dcc, html
from pathlib import Path
from dash    import callback
from dash    import Input
from dash    import Output
from glob    import glob
from dash    import ALL
from time    import sleep
from pathlib import Path

# Socket information for sending commands to CI/TO
UDP_IP = "127.0.0.1"
UDP_PORT = 5010

# IMPORTANT
import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)
#Turning off this warning so it doesn't spam the screen until it can be fixed.

# Global Values for ease of use
total_size_sc = 0.0
total_size_dl = 0.0
default_sigma = 0.42
default_dl = 5.0
onboard_path = "/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/onboard"
downlink_path = "/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/downlink"
onboard_color = '#d9ebf0'
ground_color = '#dbfcd9'

# Faster method for file sizes (takes too long to properly calculate in Python)
file_dict = {'asdp000000000': 1.306, 'asdp000000001': 0.773, 'asdp000000002': 0.538, 'asdp000000003': 1.055, 'asdp000000004': 0.403, 'asdp000000005': 0.635, 'asdp000000006': 1.240, 'asdp000000007': 0.494 }

x_coord_dd_ind = 1
y_coord_dd_ind = 7

dd_extrema = np.array([1000.,
                       1000.,
                       1000.,
                       500.,
                       500.,
                       500.,
                       20.,
                       20.,
                       20.])[[x_coord_dd_ind, y_coord_dd_ind]]

dd_names = ["bbox_area_min_pc10.0",
            "bbox_area_min_pc50.0",
            "bbox_area_min_pc90.0",
            "disp_e2e_norm_pc10.0",
            "disp_e2e_norm_pc50.0",
            "disp_e2e_norm_pc90.0",
            "speed_mean_pc10.0",
            "speed_mean_pc50.0",
            "speed_mean_pc90.0"]

dd_readable_names = ['Particle Size\n(Pixels)', # 10th percentile
                     'Particle Size\n(Pixels)', # 50th percentile
                     'Particle Size\n(Pixels)', # 90th percentile
                     'Mean Displacement\n(Pixels)', # 10th percentile
                     'Mean Displacement\n(Pixels)', # 50th percentile 
                     'Mean Displacement\n(Pixels)', # 90th percentile 
                     'Mean Speed\n(Pixels/Frame)',  # 10th percentile 
                     'Mean Speed\n(Pixels/Frame)',  # 50th percentile 
                     'Mean Speed\n(Pixels/Frame)']  # 90th percentile



def get_sues_dds(glob_path):

    experiments = glob(glob_path + "/*")
    sues, dds = [], []

    for experiment in experiments:

        sue_df = pd.read_csv(glob(os.path.join(experiment,"asdp/*_sue.csv"))[0])

        dd_df = pd.read_csv(glob(os.path.join(experiment,"asdp/*_dd.csv"))[0])

        sues.append(float(sue_df['SUE'][0]))
        #Future Warning Suppressed
        dds.append([float(dd_df[dd_names[x_coord_dd_ind]]) * dd_extrema[0],
                    float(dd_df[dd_names[y_coord_dd_ind]]) * dd_extrema[1]])

    sues = np.array(sues)
    dds = np.array(dds)
    return sues, dds

def b64_image(image_filename):
    with open(image_filename, 'rb') as f:
        image = f.read()
    return 'data:image/png;base64,' + base64.b64encode(image).decode('utf-8')


# Begin App Layout

app = dash.Dash(title='Synopsis Mission Ops Demo', external_stylesheets=[dbc.themes.BOOTSTRAP])
app.layout = html.Div(id = 'parent', children = [
    dcc.Interval(id="refresh", interval=3 * 1000, n_intervals=0),
    html.H1("SYNOPSIS Mission Ops", style={'textAlign': 'center'}),
    html.P(html.I(html.Center("Science Yield improvemeNt via Onboard Prioritization and Summary of Information System"))),
    html.Br(),
    # html.H6("Sigma", style={'textAlign': 'center'}),

    # # Sigma Slider
    # dcc.Slider(0, 1, 0.01, value=default_sigma, marks={0:'0',0.50:'0.5', 1:'1'}, tooltip={"placement": "bottom", "always_visible": True}, id='sigma_slider'),
    # html.Br(),

    # # Downlink Volume Slider
    # dbc.Row(html.H6("Downlink Data Volume (Mb)", style={'textAlign': 'center'})),
    # dcc.Slider(0, 15, 0.1, value=default_dl, marks={0:'0',15:'15'}, tooltip={"placement": "bottom", "always_visible": True}, id='downlink_slider'),
    # html.Br(),
    # html.Br(),

    # Reset and Simulate Buttons
    dbc.Row([dbc.Col(html.Button('START COSMOS', id='cosmos_button', n_clicks=0, style={'width': '220px',
                                                            'font-size': '90%',
                                                            'height': '40px',
                                                            'textAlign': 'center',
                                                        })),
            #dbc.Col(),], style={'textAlign':'center'},),         
            ], style={'textAlign':'center'},), 
    html.P(html.Center("Launch Cosmos Ground Station Software - Commands the Demo")),
  
    # # Reset and Simulate Info Alerts
    # html.Div([        
    #     dbc.Alert("Resetting Database, and Repopulating Files! Please wait for this notification to close.",
    #                             id='alerts',
    #                             is_open=False,
    #                             duration=10000,
    #                             style={'textAlign':'center'},)
    # ]),
    # html.Div([        
    #     dbc.Alert("Downlinking Priority Files Within Size Window! Please wait for this notification to close.",
    #                             id='dl_alerts',
    #                             is_open=False,
    #                             duration=5000,
    #                             style={'textAlign':'center'},)
    # ]),

    html.Hr(),
    
    # Graphical Data
    dbc.Row([

            # Onboard tiles
            dbc.Col(children=[
                dbc.Row(html.H3("Onboard Data", style={'textAlign': 'center'})),

                dbc.Row(children=[
                    dbc.Col(id="onboard_1"),
                    dbc.Col(id="onboard_2"),
                    dbc.Col(id="onboard_3"),
                    dbc.Col(id="onboard_4"),
                ], style={'padding': 10}),

                dbc.Row(children=[
                    dbc.Col(id="onboard_5"),
                    dbc.Col(id="onboard_6"),
                    dbc.Col(id="onboard_7"),
                    dbc.Col(id="onboard_8"),
                ], style={'padding': 10}),
                
            ], style={'padding': 10, 'flex': 1,'flex-direction': 'col','background-color': onboard_color}),

            # Downlink tiles
            dbc.Col(children=[
                dbc.Row(html.H3("Downlinked Data", style={'textAlign': 'center'})),

                dbc.Row(children=[
                    dbc.Col(id="downlinked_1"),
                    dbc.Col(id="downlinked_2"),
                    dbc.Col(id="downlinked_3"),
                    dbc.Col(id="downlinked_4"),
                ], style={'padding': 10}),

                dbc.Row(children=[
                    dbc.Col(id="downlinked_5"),
                    dbc.Col(id="downlinked_6"),
                    dbc.Col(id="downlinked_7"),
                    dbc.Col(id="downlinked_8"),
                ], style={'padding': 10}),

            ], style={'padding': 10, 'flex': 1,'flex-direction': 'col', 'background-color': ground_color}),

    ]),

    # ScatterPlots
    dbc.Row([

        # Onboard graph
        dbc.Col(children=[

            dbc.Row(children=[
                dcc.Graph(id='onboard-plot'),
            ], style={'padding': 10, 'flex': 1,'flex-direction': 'col','background-color': onboard_color}),

        ], style={'padding': 10, 'flex': 1,'flex-direction': 'col','background-color': onboard_color}),

        # Downlink graph
        dbc.Col(children=[

            dbc.Col(children=[
                dcc.Graph(id='downlinked-plot'),
            ], style={'padding': 10, 'flex': 1,'flex-direction': 'col', 'background-color': ground_color}),


        ], style={'padding': 10, 'flex': 1,'flex-direction': 'col', 'background-color': ground_color}),

    ]),

    dcc.Dropdown(
        options=['Hide Onboard Data', 'Show Onboard Data'],
       value='Hide Onboard Data',
       id='hide-onboard'
    ),

    # #TODO: This can probably go away.  Our demos should be close to identical now.
    # dcc.Dropdown(
    #    options=['NOS3', 'Science Slider'],
    #    value='NOS3',
    #    id='mode'
    # ),

    html.P(id='placeholder'),

])


# Launch COSMOS
@app.callback(
        Output('placeholder', 'children'),
        Input('cosmos_button', 'n_clicks'),
        prevent_initial_call=True
)
def launch_cosmos(n_clicks):
    changed_id = [p['prop_id'] for p in dash.callback_context.triggered][0]
    #print(f"This is the change that happened: {changed_id}")
    cosmos_dir = "/home/nos3/Desktop/github-nos3/gsw/cosmos"
    if 'cosmos_button' in changed_id:
        cmd = 'cd "/home/nos3/Desktop/github-nos3/gsw/cosmos" && echo "COSMOS Ground Station Launching ... " && export MISSION_NAME=$(echo "NOS3") && export PROCESSOR_ENDIANNESS=$(echo "LITTLE_ENDIAN") && ruby Launcher -c nos3_launcher.txt --system nos3_system.txt &'
        os.system(cmd)

# # Callback Alert functionality for Reset Alert
# @app.callback(
#         Output('alerts', 'is_open'),
#         Input('reset_button', 'n_clicks'),
#         Input("alerts", "is_open"),
#         prevent_initial_call=True
# )
# def show_alert(n_clicks, is_open):
#     changed_id = [p['prop_id'] for p in dash.callback_context.triggered][0]
#     #print(f"This is the change that happened: {changed_id}")
#     if 'reset_button' in changed_id:
#         return not is_open
#     return is_open

# # Callback Alert functionality for Simulate Alert
# @app.callback(
#         Output('dl_alerts', 'is_open'),
#         Input('simulate_button', 'n_clicks'),
#         Input("dl_alerts", "is_open"),
#         prevent_initial_call=True
# )
# def show_dl_alert(n_clicks, is_open):
#     changed_id = [p['prop_id'] for p in dash.callback_context.triggered][0]
#     #print(f"This is the change that happened: {changed_id}")
#     if 'simulate_button' in changed_id:
#         return not is_open
#     return is_open

# # Calback and functionality to reset downlink slider to default
# @app.callback(
#         Output('downlink_slider', 'value'),
#         Input('reset_button', 'n_clicks'),
#         prevent_initial_call=True
# )
# def reset_dl_slider(n_clicks):
#     changed_id = [p['prop_id'] for p in dash.callback_context.triggered][0]
#     #print(f"This is the change that happened: {changed_id}")
#     if 'reset_button' in changed_id:
#         return default_dl

# # Callback and functionality to reset sigma slider to default   
# @app.callback(
#         Output('sigma_slider', 'value'),
#         Input('reset_button', 'n_clicks'),
#         prevent_initial_call=True
# )
# def reset_sigma_slider(n_clicks):
#     changed_id = [p['prop_id'] for p in dash.callback_context.triggered][0]
#     #print(f"This is the change that happened: {changed_id}")
#     if 'reset_button' in changed_id:
#         update_sigma_json(default_sigma)
#         return default_sigma 


# def update_sigma_json(sigma):
#     template_path = "/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/"
#     template_loader = jinja2.FileSystemLoader(template_path)
#     template_env = jinja2.Environment(loader=template_loader)

#     html_template = 'owls_similarity_config.template'
#     html_json = 'owls_similarity_config.json'
#     template = template_env.get_template(html_template)
#     output_text = template.render({'sigma':sigma})
#     file_path = template_path + html_json
#     text_file = open(file_path, "w")
#     text_file.write(output_text)
#     text_file.close()

# Callback and Functionality for Graphical Data
@app.callback(
    [Output('onboard-plot', 'figure'),
     Output('downlinked-plot', 'figure'),
     Output('onboard_1', 'children'),
     Output('onboard_2', 'children'),
     Output('onboard_3', 'children'),
     Output('onboard_4', 'children'),
     Output('onboard_5', 'children'),
     Output('onboard_6', 'children'),
     Output('onboard_7', 'children'),
     Output('onboard_8', 'children'),
     Output('downlinked_1', 'children'),
     Output('downlinked_2', 'children'),
     Output('downlinked_3', 'children'),
     Output('downlinked_4', 'children'),
     Output('downlinked_5', 'children'),
     Output('downlinked_6', 'children'),
     Output('downlinked_7', 'children'),
     Output('downlinked_8', 'children')],
    [Input("refresh", "n_intervals"),
     Input('hide-onboard', 'value')],)
def update_output(n_intervals,hide_onboard):

    # changed_id = [p['prop_id'] for p in dash.callback_context.triggered][0]
    # #print(f"This is the change that happened: {changed_id}")

    # #Sigma Slider
    # if 'sigma_slider' in changed_id:
    #     print(f'The sigma slider was set to: {sigma_slider}')
    #     sigma = sigma_slider 
    #     update_sigma_json(sigma)

    #     message = bytearray.fromhex('18fcc00000010600')
    #     print(f'CI PRIORITIZE MESSAGE: {message}')
    #     sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #     sock.sendto(message, (UDP_IP, UDP_PORT))
        
    # else:
    #     pass

    # #Downlink Slider
    # if 'downlink_slider' in changed_id: 
    #     print(f'The downlink slider was set to: {downlink_slider}')
    #     data_volume = downlink_slider
    #     message = bytearray.fromhex('18fcc00000050900')
    
    #     int_data_volume = int(data_volume * 10)
    #     data_volume_bytes_val = int_data_volume.to_bytes(4,'little')
    #     #data_volume_bytes_val = bytes(int_data_volume.to_bytes(4, byteorder='little')).hex()
    #     full_message = message + data_volume_bytes_val
    #     print(f'CI DOWNLINK_SIZE MESSAGE: {full_message}')
    #     sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #     sock.sendto(full_message, (UDP_IP, UDP_PORT))
    # else:
    #     pass

    # #Reset Button
    # if 'reset_button' in changed_id: 
    #     message = bytearray.fromhex('18fcc00000010500')
    #     print(f'CI RESET MESSAGE: {message}')
    #     sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #     sock.sendto(message, (UDP_IP, UDP_PORT))

    #     print(f'Please wait ~10 seconds to allow data to be reset and repopulated!')
    #     sleep(10)
    #     print(f'Reset Complete.  Repopulating Windows')
    # else:
    #     pass

    # #Simulate Button
    # if 'simulate_button' in changed_id: 
    #     message = bytearray.fromhex('18fcc00000010700')
    #     print(f'CI SIMULATE MESSAGE: {message}')
    #     sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #     sock.sendto(message, (UDP_IP, UDP_PORT))
    #     sleep(5)
    #     print(f'Downlink Complete.  Repopulating Windows')
    #     else:
    #         pass

    ob_files_list = os.listdir(onboard_path)
    ob_file_sizes = []
    ob_total_size = 0.0
        
    for file in ob_files_list:
        size = file_dict[file]
        ob_file_sizes.append(size)
        ob_total_size += size
        
    dl_files_list = os.listdir(downlink_path)
    dl_file_sizes = []
    dl_total_size = 0.0
        
    for file in dl_files_list:
        size = file_dict[file]
        dl_file_sizes.append(size)
        dl_total_size += size

    downlinks = dl_files_list
    onboards = ob_files_list

    cl = []
    obs_count = 0

    # Fill onboard
    for x in range(0,8):
        try:
            observation = onboards[x]
            obs_path = "assets/" + observation
            os.chdir("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/")
            cwd = os.getcwd()
            if hide_onboard == "Hide Onboard Data":
                mhi_file = glob(os.path.join(obs_path, "validate/unknown.jpg"))
            else:
                mhi_file = glob(obs_path + "/validate/*_mhi.jpg")
            sue_file = glob(os.path.join(obs_path, "asdp/*_sue.csv"))[0]
            dqe_file = glob(os.path.join(obs_path, "asdp/*_dqe.csv"))[0]
            with open(sue_file) as file:
                sue = float([line.rstrip() for line in file][-1])
            with open(dqe_file) as file:
                dqe = float([line.rstrip() for line in file][-1])
            file_size = file_dict[observation]
            tmp = dbc.Card([
                    dbc.CardHeader(f"{observation}"),
                    dbc.CardImg(src=mhi_file, top=True),
                    dbc.CardBody([
                        html.P(f"SUE = {sue:.2f}"),
                        html.P(f"DQE = {dqe:.2f}"),
                        html.P(f"Size = {file_size:.2f} Mb")
                    ], style={'fontSize': 14,
                              'textAlign': 'center'}
                ),
            ],color="primary", outline=True)

            obs_count += 1

        except:
            tmp = html.P()

        cl.append(tmp)

    # Fill downlinked
    dl_count = 0
    for x in range(0,8):

        try:
            observation = downlinks[x]
            obs_path = "assets/" + observation
            os.chdir("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/")
            #observation_name = observation.strip('/').split('/')[-1]

            mhi_file = glob(obs_path + "/validate/*_mhi.jpg")
            sue_file = glob(os.path.join(obs_path, "asdp/*_sue.csv"))[0]
            dqe_file = glob(os.path.join(obs_path, "asdp/*_dqe.csv"))[0]
            with open(sue_file) as file:
                sue = float([line.rstrip() for line in file][-1])
            with open(dqe_file) as file:
                dqe = float([line.rstrip() for line in file][-1])

            file_size = file_dict[observation]
            tmp = dbc.Card([
                    dbc.CardHeader(f"{observation}"),
                    dbc.CardImg(src=mhi_file, top=True),
                    dbc.CardBody([
                        html.P(f"SUE = {sue:.2f}"),                        
                        html.P(f"DQE = {dqe:.2f}"),
                        html.P(f"Size = {file_size:.2f} Mb"),
                    ], style={'fontSize': 14,
                              'textAlign': 'center'}
                ),
            ],color="primary", outline=True)

            dl_count += 1

        except:
            tmp = html.P()

        cl.append(tmp)

    onboard_sues, onboard_dds = get_sues_dds(onboard_path)
    downlinked_sues, downlinked_dds = get_sues_dds(downlink_path)
    
    # RB Added Logic to account for no onboard data, or no data having been downlinked.
    downlink_dd_size_itc = len(downlinked_dds)
    onboard_dd_size_itc = len(onboard_dds)
    downlink_sue_size = len(downlinked_sues);
    onboard_sue_size = len(onboard_sues)
    
    if(downlink_dd_size_itc > 0 and onboard_dd_size_itc > 0):
        x_min = min(list(onboard_dds[:,0]) + list(downlinked_dds[:,0]))
        x_max = max(list(onboard_dds[:,0]) + list(downlinked_dds[:,0]))
        y_min = min(list(onboard_dds[:,1]) + list(downlinked_dds[:,1]))
        y_max = max(list(onboard_dds[:,1]) + list(downlinked_dds[:,1]))
    elif(downlink_dd_size_itc <= 0 and onboard_dd_size_itc > 0):
        x_min = min(list(onboard_dds[:,0]))
        x_max = max(list(onboard_dds[:,0]))
        y_min = min(list(onboard_dds[:,1]))
        y_max = max(list(onboard_dds[:,1]))
    elif(downlink_dd_size_itc > 0 and onboard_dd_size_itc <= 0):
        x_min = min(list(downlinked_dds[:,0]))
        x_max = max(list(downlinked_dds[:,0]))
        y_min = min(list(downlinked_dds[:,1]))
        y_max = max(list(downlinked_dds[:,1]))
    else:
        x_min = 0
        x_max = 0
        y_min = 0
        y_max = 0

    if(downlink_sue_size > 0 and onboard_sue_size > 0):
        sue_min = min(list(onboard_sues) + list(downlinked_sues))
        sue_max = max(list(onboard_sues) + list(downlinked_sues))
    if(downlink_sue_size <= 0 and onboard_sue_size > 0):
        sue_min = min(list(onboard_sues))
        sue_max = max(list(onboard_sues))
    if(downlink_sue_size > 0 and onboard_sue_size <= 0):        
        sue_min = min(list(downlinked_sues))
        sue_max = max(list(downlinked_sues))
    else:
        sue_min = 0
        sue_max = 0
        
    # End RB
        

    x_margin = abs(x_max - x_min) * 0.25
    y_margin = abs(y_max - y_min) * 0.25
    x_min -= x_margin
    x_max += x_margin
    y_min -= y_margin
    y_max += y_margin
    if x_min < 0:
        x_min = 0

    if y_min < 0:
        y_min = 0
      
    #RB Added Logic to update scatterplot when onboard or downlink is empty  
    if(onboard_dd_size_itc > 0):
        onboard_fig = px.scatter(x=onboard_dds[:,0], y=onboard_dds[:,1], color=onboard_sues, color_continuous_scale='viridis', range_color=(sue_min,sue_max))
        onboard_fig.update_layout(title={'text': "Prioritization on OWLS Data",'y':0.95,'x':0.5,'xanchor': 'center','yanchor': 'top'})
        onboard_fig.update_layout(xaxis_title=dd_readable_names[x_coord_dd_ind], yaxis_title=dd_readable_names[y_coord_dd_ind])
        onboard_fig.update_layout(coloraxis_colorbar=dict(title="SUE (Sci. Utility Est.)"))
        onboard_fig.update_traces(marker=dict(size=12, line=dict(width=1, color='black')), selector=dict(mode='markers'))
        onboard_fig.update_layout(coloraxis_colorbar_title_side="right")
        onboard_fig.update_layout(xaxis_range=[x_min,x_max], yaxis_range=[y_min,y_max])
    else:
        onboard_fig = {}
    
    if(downlink_dd_size_itc > 0):
        downlinked_fig = px.scatter(x=downlinked_dds[:,0], y=downlinked_dds[:,1], color=downlinked_sues, color_continuous_scale='viridis', range_color=(sue_min,sue_max))
        downlinked_fig.update_layout(title={'text': "Prioritization on OWLS Data",'y':0.95,'x':0.5,'xanchor': 'center','yanchor': 'top'})
        downlinked_fig.update_layout(xaxis_title=dd_readable_names[x_coord_dd_ind], yaxis_title=dd_readable_names[y_coord_dd_ind])
        downlinked_fig.update_layout(coloraxis_colorbar=dict(title="SUE (Sci. Utility Est.)"))
        downlinked_fig.update_traces(marker=dict(size=12, line=dict(width=1, color='black')), selector=dict(mode='markers'))
        downlinked_fig.update_layout(coloraxis_colorbar_title_side="right")
        downlinked_fig.update_layout(xaxis_range=[x_min,x_max], yaxis_range=[y_min,y_max])
    else:
        downlinked_fig = {}
    #End RB
    
    return onboard_fig, downlinked_fig, cl[0], cl[1], cl[2], cl[3], cl[4], cl[5], cl[6], cl[7], cl[8], cl[9], cl[10], cl[11], cl[12], cl[13], cl[14], cl[15]


if __name__ == "__main__":
    app.run_server(debug=True)

