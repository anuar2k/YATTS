<template>
  <h2 class="head bg-1">
    YATTS.Configurator
  </h2>
  <div class="container">
    <div class="row">
      <div class="col">
        <TelemVarSelector 
          v-bind:config="config" 
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
import TelemVarSelector from '@/components/TelemVarSelector';

import TelemVarDefinitions from '@/utils/TelemVarDefinitions';
import { GetInitialConfig } from '@/utils/TelemVarConfigUtil';

export default {
  name: 'App',
  setup() {
    const config = GetInitialConfig(TelemVarDefinitions);

    const toggleShow = subtree => {
      subtree.show = !subtree.show;
    }

    const toggleSelected = variable => {
      variable.selected = !variable.selected;
    }

    return { config, toggleShow, toggleSelected }
  },
  components: {
    TelemVarSelector 
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
