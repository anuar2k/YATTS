<template>
  <h4 class="bg-primary text-white text-center p-2">
    YATTS.Configurator
  </h4>
  <div class="container-fluid">
    <div class="row">
      <div class="col">
        <TelemVarCatalogue v-bind:state="state" />
      </div>
      <div class="col">

      </div>
    </div>
  </div>
</template>

<script>
import TelemVarCatalogue from './components/TelemVarCatalogue';

import TelemVarDefinitions from './utils/TelemVarDefinitions';
import { GenerateInitialState } from './utils/TelemVarStateUtil';

export default {
  name: 'App',
  data() {
    return {
      state: GenerateInitialState(TelemVarDefinitions)
    }
  },
  components: { TelemVarCatalogue }
}
</script>
