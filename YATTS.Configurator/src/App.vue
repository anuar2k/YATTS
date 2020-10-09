<template>
  <div class="head bg-1">
    YATTS.Configurator
  </div>
  <div class="container">
    <div class="row mb-2">
      <div class="col">
        <TelemVarCard />
      </div>
    </div>
    <div class="row">
      <div class="col">
        <TelemVarSelector 
          v-bind:config="config" 
          v-on:toggle-show="toggleShow"
        />
      </div>
      <div class="col desc">
        <component v-bind:is="config[0].groups[0].variables[0].value.def.desc" />
      </div>
    </div>
  </div>
</template>

<script>
import TelemVarCard from '@/components/TelemVarCard';
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

    return { config, toggleShow }
  },
  components: {
    TelemVarCard,
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
