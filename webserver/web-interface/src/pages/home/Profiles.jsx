import React, { useState, useEffect, useMemo } from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions, Paper, TextareaAutosize, Alert,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import QrCodeIcon from '@mui/icons-material/QrCode';
import UploadFileIcon from '@mui/icons-material/UploadFile';
import Grid from '@mui/material/Unstable_Grid2';
import ProfileChart from '../../components/chart/ProfileChart';
import { Profile, NamedProfile, GlobalStopConditions, createCurveStyleFromString, createPhaseTypeFromString, PhaseStopConditions, PhaseTypes, Phase, Transition } from '../../models/profile';
import axios from 'axios';
import { DataGrid } from '@mui/x-data-grid';
import BuildProfileEditor from '../../components/buildProfile/BuildProfileEditor';

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
    const values = profile.phases.flatMap((phase, phaseIndex) => {
      const phaseValues = JSON.parse(JSON.stringify(defaultProfileValues))
      console.log(phase)
      for (let index = 0; index < defaultProfileValues.length; index++) {
        phaseValues[index].id = String((phaseIndex * defaultProfileValues.length + 1 * index + 1))
      }
      phaseValues[0].value = phase.type
      phaseValues[1].value = phase.target.start
      phaseValues[2].value = phase.target.end
      phaseValues[3].value = phase.target.curve
      phaseValues[4].value = phase.target.time / 1000
      phaseValues[5].value = phase.restriction
      phaseValues[6].value = phase.stopConditions.time
      phaseValues[7].value = phase.stopConditions.weight
      phaseValues[8].value = phase.stopConditions.pressureAbove
      phaseValues[9].value = phase.stopConditions.pressureBelow
      phaseValues[10].value = phase.stopConditions.waterPumpedInPhase
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
      {BuildProfileEditor(theme, handleRemoveAll, handleRemoveRow, handleAddRow, globalStopConditions, setGlobalStopConditions, phases, handleSelectChange, handleApply)}
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
