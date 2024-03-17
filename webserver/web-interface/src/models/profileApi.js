import axios from "axios";
import { Profile } from "./profile";

export async function getProfiles() {
  return axios.get('/api/profiles/list')
    .then(({ data }) => data);
}

export async function setDefaultProfile(profile) {
  return axios.post('/api/profiles/setDefault', profile)
    .then(({ data }) => data);
}

export async function saveProfile(namedProfile) {
  return axios.post('/api/profiles/save', namedProfile)
    .then(({ data }) => data);
}