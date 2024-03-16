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
import InputLabel from '@mui/material/InputLabel';
import FormControl from '@mui/material/FormControl';
import List from '@mui/material/List';
import Table from '@mui/material/Table';

export default function Profiles() {
  const theme = useTheme();

  const defaultProfileValues = [
    { id: 1, type: 'selectType', value: '0' },
    { id: 2, type: 'targetStart', value: '' },
    { id: 3, type: 'targetEnd', value: '' },
    { id: 4, type: 'transitionType', value: '0' },
    { id: 5, type: 'transitionTime', value: '' },
    { id: 6, type: 'restriction', value: '' },
    { id: 7, type: 'stopType', value: '0' },
    { id: 8, type: 'stopValue', value: '' },
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
  const [nextId, setNextId] = useState(defaultProfileValues.length);
  
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
      ...defaultProfileValues.map((element) => ({ ...element, ['id']: element.id + nextId}))
    ];
    setElements(newElements);
    setNextId(nextId + defaultProfileValues.length);
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
    for (let i = 0; i < defaultProfileValues.length; i++) {
      newElements.pop();
    }
    setElements(newElements);
  };

  const handleRemoveAll = () => {
    setElements(defaultProfileValues);
    setNextId(defaultProfileValues.length);
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
                      <Grid container xs={12} columnSpacing={1} rowSpacing={3}>
                        {phases.map((element) => {
                          switch (element.type) {
                            case 'selectType':
                              return (
                                <Grid item xs={2}>
                                  <FormControl sx={{ minWidth: 150 }}>
                                  <InputLabel>Type</InputLabel>
                                  <Select
                                    value={element.value}
                                    defaultValue='Preinfusion'
                                    onChange={(event) => handleSelectChange(event, element.id)}
                                    displayEmpty={false}
                                    label="Phase"
                                  >
                                    <MenuItem value='0'>Flow</MenuItem>
                                    <MenuItem value="1">Pressure</MenuItem>
                                  </Select>
                                  </FormControl>
                                </Grid>
                              );
                            case 'targetStart':
                            case 'targetEnd':
                            case 'transitionTime':
                            case 'restriction':
                            case 'stopValue':
                              return (
                                <Grid item xs={element.type === 'restriction' ? 8 : element.type === 'targetEnd' ? 4 : 2}>
                                  <TextField
                                    value={element.value}
                                    // label={element.type === 'targetStart' ? 'Target start' : element.type === 'targetEnd' ? 'Target end' : element.type === 'transitionTime' ? 'Transition time' : 'Restriction'}
                                    label={element.type}
                                    onChange={(event) => handleSelectChange(event, element.id)}
                                  />
                                </Grid>
                              );
                            case 'transitionType':
                              return (
                                <Grid item xs={2}>
                                  <FormControl sx={{ minWidth: 150 }}>
                                    <InputLabel>Transition curve</InputLabel>
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
                                  </FormControl>
                                </Grid>
                              );
                            case 'stopType':
                              return (
                                <Grid item xs={2}>
                                  <FormControl sx={{ minWidth: 150 }}>
                                    <InputLabel>Stop on</InputLabel>
                                  <Select
                                    value={element.value}
                                    defaultValue='Time'
                                    onChange={(event) => handleSelectChange(event, element.id)}
                                    displayEmpty={false}
                                    label="Transition curve"
                                  >
                                    <MenuItem value='0'>Time</MenuItem>
                                    <MenuItem value="1">Pressure Above</MenuItem>
                                    <MenuItem value="3">Pressure Below</MenuItem>
                                    <MenuItem value="4">Flow Above</MenuItem>
                                    <MenuItem value="5">Flow Below</MenuItem>
                                    <MenuItem value="6">Weight reached</MenuItem>
                                    <MenuItem value="6">Water pumped</MenuItem>
                                  </Select>
                                  </FormControl>
                                </Grid>
                              );
                            default:
                              return null;
                          }
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
