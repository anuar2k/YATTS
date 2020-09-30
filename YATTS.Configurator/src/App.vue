<template>
  <h2 class="head bg-1">
    YATTS.Configurator
  </h2>
  <div class="container">
    <div class="row">
      <div class="col">
        <TelemVarCatalogue 
          v-bind:state="state" 
          v-on:toggle-show="toggleShow"
          v-on:toggle-selected="toggleSelected" 
        />
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
  methods: {
    toggleShow(subtree) {
      subtree.show = !subtree.show;
    },
    toggleSelected(variable) {
      variable.selected = !variable.selected;
    }
  },
  components: {
    TelemVarCatalogue 
  }
}
</script>

<style lang="scss" scoped>
  .head {
    color: white;
    text-align: center;
    padding: .5rem;
    font-size: 2rem;
    margin-bottom: 0.5rem;
    line-height: 1.2;
    margin-top: 0;
  }
</style>
