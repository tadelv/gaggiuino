import {
  Card, Container, useTheme, Typography, CardContent, CardActions, Paper, TextareaAutosize, Alert, Grid, FormControl,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import AutoGraphIcon from '@mui/icons-material/AutoGraph';
import DeleteIcon from '@mui/icons-material/Delete';
import TextField from '@mui/material/TextField';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';
import InputLabel from '@mui/material/InputLabel';
import { GridSeparatorIcon } from '@mui/x-data-grid';

export default function BuildProfileEditor(
  theme, 
  handleRemoveAll, 
  handleRemoveRow, 
  handleAddRow, 
  globalStopConditions, 
  setGlobalStopConditions, 
  phases, 
  handleSelectChange, 
  handleApply,
  currentProfileName,
  setCurrentProfileName
  ) {
  return <Grid container rowSpacing={1} columns={{ xs: 1, sm: 1 }}>
        {/* <Grid item xs={1}> */}
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
                <Grid container rowSpacing={3} spacing={2}>
                  <Grid container xs={12} m={5}>
              <Grid item xs={3}>
                <TextField label="Profile name" value={currentProfileName} onChange={(event) => {
                  setCurrentProfileName(event.target.value)
                }} />
              </Grid>
                    <Grid item xs={3}>
                      <TextField label="Time(s)" value={globalStopConditions.time > 0 ? globalStopConditions.time : ""} onChange={(event) => {
                        setGlobalStopConditions({ ...globalStopConditions, ['time']: event.target.value });
                      }} />
                    </Grid>
                    <Grid item xs={3}>
                      <TextField label="Weight(g)" value={globalStopConditions.weight > 0 ? globalStopConditions.weight : ""} onChange={(event) => {
                        setGlobalStopConditions({ ...globalStopConditions, ['weight']: event.target.value });
                      }} />
                    </Grid>
                    <Grid item xs={3}>
                      <TextField label="Total water(ml)" value={globalStopConditions.totalWaterPumped > 0 ? globalStopConditions.totalWaterPumped : ""} onChange={(event) => {
                        setGlobalStopConditions({ ...globalStopConditions, ['totalWaterPumped']: event.target.value });
                      }} />
                    </Grid>
                  </Grid>
                  <Grid container xs={12} columnSpacing={1} rowSpacing={2} marginLeft={1}>
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
                                onChange={(event) => handleSelectChange(event, element.id)} />
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
        {/* </Grid> */}
      </Grid>
}