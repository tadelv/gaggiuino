import React, { useState, useEffect, useMemo } from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions, Paper, TextareaAutosize, Alert,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import QrCodeIcon from '@mui/icons-material/QrCode';
import UploadFileIcon from '@mui/icons-material/UploadFile';
import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import AutoGraphIcon from '@mui/icons-material/AutoGraph';
import DeleteIcon from '@mui/icons-material/Delete';
import TextField from '@mui/material/TextField';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';
// import InputAdornment from '@mui/material/InputAdornment';
import Grid from '@mui/material/Unstable_Grid2';
import ProfileChart from '../../components/chart/ProfileChart';
import { Profile, NamedProfile, GlobalStopConditions } from '../../models/profile';
import axios from 'axios';
import { DataGrid } from '@mui/x-data-grid';
import { elements } from 'chart.js';


export default function Profiles() {
  const theme = useTheme();

  const defaultProfileValues = [
    { id: 1, type: 'selectType', value: '0' },
    { id: 2, type: 'targetStart', value: '' },
    { id: 3, type: 'targetEnd', value: '' },
    { id: 4, type: 'transitionType', value: '0' },
    { id: 5, type: 'transitionTime', value: '' },
    { id: 6, type: 'restriction', value: '' },
  ]

  const columns = [
    // { field: 'id', headerName: 'ID' },
    { field: 'name', headerName: 'Name' },
  ];

  const [globalStopConditions, setGlobalStopConditions] = useState(
    { time: 0, weight: 0, totalWaterPumped: 0 }
  )
  const handleOnChangeGlobalStopConditions = event => {
    const { name, value } = event.target;
    console.log("setting " + name + "and " + value)
    setGlobalStopConditions({ ...globalStopConditions, [name]: 0+value});
  };

  const [phases, setElements] = useState(defaultProfileValues);
  const [nextId, setNextId] = useState(7);
  
  const [profiles, setProfiles] = useState(
    [new NamedProfile([])]
  );

  const profileRows = useMemo(
    () => profiles.map((profile, index) => ({id: index, name: profile.name})),
    [profiles]
  )

  async function getProfiles() {
    return axios.get('/api/profiles/list')
      .then(({ data }) => data);
  }

  async function loadProfiles() {
    const newProfiles = await getProfiles();
    setProfiles(newProfiles);
  }

  useEffect(() => {
    // Fetch objects from the server when the component mounts
   loadProfiles();
  }, []);

  const handleAddRow = () => {
    const newElements = [
      ...phases,
      ...defaultProfileValues
    ];
    setElements(newElements);
    setNextId(nextId + 6);
    console.log("after update: ", newElements)
  };

  const [error, setError] = useState(null);
  const [profile, setProfile] = useState(new Profile([]));

  const updateProfile = (value) => {
    try {
      setProfile(Profile.parse(JSON.parse(value)));
      setError(undefined);
    } catch (er) {
      setError(er.message);
    }
  };

  const handleRemoveRow = () => {
    const newElements = [...phases];
    for (let i = 0; i < 6; i++) {
      newElements.pop();
    }
    setElements(newElements);
  };

  const handleRemoveAll = () => {
    setElements(defaultProfileValues);
    setNextId(7);
  };

  const handleSelectChange = (event, id) => {
    const updatedElements = phases.map((element) => {
      if (element.id === id) {
        return { ...element, value: event.target.value };
      }
      return element;
    });
    setElements(updatedElements);
    console.log("after update: ", JSON.stringify(updatedElements))
  };

  return (
    <div>
      <Container sx={{ mt: theme.spacing(2) }}>
        <Card sx={{ mt: theme.spacing(2) }}>
          <Grid container columns={{ xs: 1, sm: 2 }}>
            <Grid item xs={1}>
              <CardContent>
                <Typography gutterBottom variant="h5">
                  Load Profile
                </Typography>
              </CardContent>
              <CardActions>
                <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                  <input hidden accept=".bin" type="file" />
                  <UploadFileIcon fontSize="large" />
                </IconButton>
                <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label">
                  <input hidden accept=".png" type="file" />
                  <QrCodeIcon fontSize="large" />
                </IconButton>
              </CardActions>
            </Grid>
          </Grid>
        </Card>
      </Container>
      <Container sx={{ mt: theme.spacing(2) }}>
        <Card sx={{ mt: theme.spacing(2) }}>
          <Grid container columns={{ xs: 1, sm: 1 }}>
            <Grid item xs={1}>
              <CardContent>
                <Typography gutterBottom variant="h5">
                  Build Profile
                  <IconButton style={{ float: 'right' }} onClick={handleRemoveAll} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <DeleteIcon fontSize="large" />
                  </IconButton>
                  <IconButton style={{ float: 'right' }} onClick={handleRemoveRow} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <RemoveIcon fontSize="large" />
                  </IconButton>
                  <IconButton style={{ float: 'right' }} onClick={handleAddRow} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <AddIcon fontSize="large" />
                  </IconButton>
                  <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <AutoGraphIcon fontSize="large" />
                  </IconButton>
                  <div>
                    <Grid container spacing={2}>
                      <Grid container spacing={2}>
                        <Grid item xs={4}>
                      <TextField  label="Time(s)" value={globalStopConditions.time > 0 ? globalStopConditions.time : ""} onChange={(event) => {
                        setGlobalStopConditions({ ...globalStopConditions, ['time']: event.target.value })
                      }} />
                        </Grid> 
                        <Grid item xs={4}>
                      <TextField label="Weight(g)" value={globalStopConditions.weight > 0 ? globalStopConditions.weight : ""} onChange={(event) => {
                        setGlobalStopConditions({ ...globalStopConditions, ['weight']: event.target.value })
                      }} />
                        </Grid>
                        <Grid item xs={4}>
                      <TextField label="Total water(ml)" value={globalStopConditions.totalWaterPumped > 0 ? globalStopConditions.totalWaterPumped : ""} onChange={(event) => {
                        setGlobalStopConditions({ ...globalStopConditions, ['totalWaterPumped']: event.target.value })
                      }} />
                        </Grid>
                      </Grid>
                      <Grid container xs={12} spacing={0}> 
                      {phases.map((element) => {
                        if (element.type === 'selectType') {
                          return (
                            <Grid item xs={2} key={element.id}>
                              <Select
                                value={element.value}
                                defaultValue='Preinfusion'
                                onChange={(event) => handleSelectChange(event, element.id)}
                                displayEmpty={false}
                                label="Phase"
                              >
                                <MenuItem value='0'>Flow</MenuItem>
                                <MenuItem value="1">Pressure</MenuItem>
                                {/* <MenuItem value="3">Flow</MenuItem>
                                <MenuItem value="4">Pressure</MenuItem> */}
                              </Select>
                            </Grid>
                          );
                        }
                        if (element.type === 'targetStart') {
                          return (
                            <Grid item xs={2} key={element.id}>
                              <TextField value={element.value} label={'Target start'} onChange={(event) => handleSelectChange(event, element.id)}/>
                            </Grid>
                          );
                        }
                        if (element.type === 'targetEnd') {
                          return (
                            <Grid item xs={2} key={element.id}>
                              <TextField value={element.value} label={'Target end'} onChange={(event) => handleSelectChange(event, element.id)} />
                            </Grid>
                          );
                        }
                        if (element.type === 'transitionTime') {
                          return (
                            <Grid item xs={2} key={element.id}>
                              <TextField value={element.value} label={'Transition time'} onChange={(event) => handleSelectChange(event, element.id)} />
                            </Grid>
                          );
                        }
                        if (element.type === 'transitionType') {
                          return (
                            <Grid item xs={2} key={element.id}>
                              <Select
                                value={element.value}
                                defaultValue='Linear'
                                onChange={(event) => handleSelectChange(event, element.id)}
                                displayEmpty={false}
                                label="Transition curve"
                              >
                                <MenuItem value='0'>Linear</MenuItem>
                                <MenuItem value="1">Instant</MenuItem>
                                <MenuItem value="3">Ease In</MenuItem>
                                <MenuItem value="4">Ease Out</MenuItem>
                                <MenuItem value="5">Ease In-Out</MenuItem>
                              </Select>
                            </Grid>
                          );
                        }
                        if (element.type === 'restriction') {
                          return (
                            <Grid item xs={2} key={element.id}>
                              <TextField value={element.value} label={'Restriction'} onChange={(event) => handleSelectChange(event, element.id)} />
                            </Grid>
                          );
                        }
                        return null;
                      })}
                      </Grid>
                    </Grid>
                  </div>
                  {/* {inputList} */}
                </Typography>
              </CardContent>
            </Grid>
          </Grid>
        </Card>
      </Container>
      <Container sx={{ mt: theme.spacing(2) }}>
        <Paper sx={{ mt: theme.spacing(2), p: theme.spacing(2) }}>
          <Typography variant="h5" sx={{ mb: theme.spacing(2) }}>
            Profile syntax playground
          </Typography>
          <Grid container columns={{ xs: 1, sm: 3 }} spacing={2}>
            <Grid item xs={1} sm={3}>
              <Alert severity={error ? 'error' : 'success'}>
                {error || 'Nice syntax!'}
              </Alert>
            </Grid>
            <Grid item xs={1} sm={1}>
              <TextareaAutosize
                minRows={15}
                onChange={(evt) => updateProfile(evt.target.value)}
                style={{ width: '100%', backgroundColor: theme.palette.background.paper, color: theme.palette.text.secondary }}
              >
              </TextareaAutosize>
            </Grid>
            <Grid item xs={1} sm={2} position="relative" height="400">
              <ProfileChart profile={profile} />
            </Grid>
          </Grid>
        </Paper>
      </Container>
      <Container>
        <Paper sx={{ mt: theme.spacing(2), p: theme.spacing(2) }}>
          {/* <Grid container spacing={2}>
            {profiles.map((profile, index) => (
              <li key={index} onClick={() => updateProfile(JSON.stringify(profile.profile))}>
                {profile.name}
                </li>
            ))}
          </Grid> */}
          <div style={{ height: 300, width: '100%' }}>
          <DataGrid
            component={Typography}
            rows={
              profileRows
            }
            columns={columns}
            initialState={{
              pagination: {
                paginationModel: {
                  pageSize: 5,
                },
              },
            }}
            pageSizeOptions={[]}
            // checkboxSelection
            disableRowSelectionOnClick
            disableColumnSelector
            onRowClick={
              (params, event, details) => {
                updateProfile(JSON.stringify(profiles[params.row.id].profile))
              }
            }
          />
          </div>
        </Paper>
      </Container>
    </div>
  );
}
