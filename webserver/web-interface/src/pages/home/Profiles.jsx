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
import { Profile, NamedProfile, GlobalStopConditions, createCurveStyleFromString, createPhaseTypeFromString, PhaseStopConditions, PhaseTypes, Phase, Transition } from '../../models/profile';
import axios from 'axios';
import { DataGrid } from '@mui/x-data-grid';
import InputLabel from '@mui/material/InputLabel';
import FormControl from '@mui/material/FormControl';

export default function Profiles() {
  const theme = useTheme();

  const defaultProfileValues = [
    { id: 1, type: 'selectType', value: 'FLOW' },
    { id: 2, type: 'targetStart', value: '' },
    { id: 3, type: 'targetEnd', value: '' },
    { id: 4, type: 'transitionType', value: 'INSTANT' },
    { id: 5, type: 'transitionTime', value: '' },
    { id: 6, type: 'restriction', value: '' },
    { id: 7, type: 'stopTime', value: '' },
    { id: 8, type: 'stopWeightAbove', value: '' },
    { id: 9, type: 'stopPressureAbove', value: '' },
    { id: 10, type: 'stopPressureBelow', value: '' },
    { id: 11, type: 'stopWater', value: '' },
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

  const handleApply = (event) => {
    console.log("applying")
    let currentPhaseValues = {}; // Object to accumulate values for the current Phase
    const newPhases = [];
    let globalStopConditions;

    phases.forEach(item => {
      switch (item.type) {
        case 'selectType':
          // If currentPhaseValues is not empty, create a Phase object and push it to newPhases array
          if (Object.keys(currentPhaseValues).length !== 0) {
            const phase = createPhaseFromValues(currentPhaseValues);
            newPhases.push(phase);
            currentPhaseValues = {}; // Reset currentPhaseValues for the next Phase
          }
          // Set type for the new Phase
          currentPhaseValues.type = createPhaseTypeFromString(item.value);
          break;
        case 'targetStart':
          currentPhaseValues.targetStart = parseFloat(item.value);
          break;
        case 'targetEnd':
          currentPhaseValues.targetEnd = parseFloat(item.value);
          break;
        case 'transitionType':
          currentPhaseValues.transitionType = createCurveStyleFromString(item.value);
          break;
        case 'transitionTime':
          currentPhaseValues.transitionTime = parseFloat(item.value) * 1000;
          break;
        case 'restriction':
          currentPhaseValues.restriction = parseFloat(item.value);
          break;
        case 'stopTime':
          currentPhaseValues.stopTime = parseFloat(item.value) * 1000;
          break;
        case 'stopWeightAbove':
          currentPhaseValues.stopWeightAbove = parseFloat(item.value);
          break;
        case 'stopPressureAbove':
          currentPhaseValues.stopPressureAbove = parseFloat(item.value);
          break;
        case 'stopPressureBelow':
          currentPhaseValues.stopPressureBelow = parseFloat(item.value);
          break;
        case 'stopWater':
          currentPhaseValues.stopWater = parseFloat(item.value);
          break;
        default:
          // Handle default case if necessary
          break;
      }
    });
    
    if (Object.keys(currentPhaseValues).length !== 0) {
      const phase = createPhaseFromValues(currentPhaseValues);
      newPhases.push(phase);
    }
    console.log(newPhases)

    const newProfile = new Profile(newPhases, globalStopConditions)
    setProfile(newProfile)
  }

  function createPhaseFromValues(values) {
    // Create Phase object using the accumulated values
    const conditions = {
      time: values.stopTime,
      pressureAbove: values.stopPressureAbove,
      pressureBelow: values.stopPressureBelow,
      weight: values.stopWeightAbove,
      waterPumpedInPhase: values.stopWater
    }
    
    return new Phase(
      values.type,
      new Transition(values.targetStart, values.targetEnd, values.transitionType, values.transitionTime),
      values.restriction,
      conditions
      // new PhaseStopConditions({[values.stopType]: values.stopValue})
    );
  }

  function createProfileValuesFromProfile(profile) {
    const values = profile.phases.flatMap((phase) => {
      const phaseValues = JSON.parse(JSON.stringify(defaultProfileValues))
      console.log(phase)
      phaseValues[0].value = phase.type
      phaseValues[1].value = phase.target.start
      phaseValues[2].value = phase.target.end
      phaseValues[3].value = phase.target.curve
      phaseValues[4].value = phase.target.time / 1000
      phaseValues[5].value = phase.restriction
      return phaseValues 
    })
    console.log(values)
    setElements(values)
  }
  
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
    setGlobalStopConditions({})
    handleApply()
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
                                      <MenuItem value='FLOW'>Flow</MenuItem>
                                      <MenuItem value="PRESSURE">Pressure</MenuItem>
                                  </Select>
                                  </FormControl>
                                </Grid>
                              );
                            case 'targetStart':
                            case 'targetEnd':
                            case 'transitionTime':
                            case 'restriction':
                            case 'stopTime':
                            case 'stopWeightAbove':
                            case 'stopPressureAbove':
                            case 'stopPressureBelow':
                            case 'stopWater':
                              return (
                                <Grid item xs={element.type === 'restriction' ? 2 : element.type === 'targetEnd' ? 4 : 2}>
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
                                      defaultValue='LINEAR'
                                    onChange={(event) => handleSelectChange(event, element.id)}
                                    displayEmpty={false}
                                    label="Transition curve"
                                  >
                                      <MenuItem value='LINEAR'>Linear</MenuItem>
                                      <MenuItem value="INSTANT">Instant</MenuItem>
                                      <MenuItem value="EASE_IN">Ease In</MenuItem>
                                      <MenuItem value="EASE_OUT">Ease Out</MenuItem>
                                      <MenuItem value="EASE_IN_OUT">Ease In-Out</MenuItem>
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
                  <div>
                    <IconButton onClick={handleApply}>Apply</IconButton>
                  </div>
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
                value={JSON.stringify(profile)}
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
                // setProfile(profiles[params.row.id.profile].profile)
                createProfileValuesFromProfile(profiles[params.row.id].profile)
              }
            }
          />
          </div>
        </Paper>
      </Container>
    </div>
  );
}
