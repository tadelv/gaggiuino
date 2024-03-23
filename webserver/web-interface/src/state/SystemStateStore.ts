import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { OperationMode, SystemState } from '../models/models';

interface SystemStateStore {
  systemState: SystemState,
  updateLocalSystemState: (newState: SystemState) => void,
}

const useSystemStateStore = create<SystemStateStore>()(
  devtools(
    (set) => ({
      systemState: {
        startupInitFinished: false,
        operationMode: OperationMode.BREW_AUTO,
        tofReady: false,
        isSteamForgottenON: false,
        scalesPresent: false,
        timeAlive: 0,
        descaleProgress: 0,
        tarePending: false,
      },
      updateLocalSystemState: (newState: SystemState) => set(() => ({ systemState: newState })),
    }),
  ),
);

export default useSystemStateStore;
