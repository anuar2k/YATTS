<template>
  <h4 class="bg-primary text-white text-center p-2">
    YATTS.Configurator
  </h4>
  <p>{{ TelemVarDefinitions }}</p>
  <YattsAccordion v-bind:elements="AccordionElements"/>
</template>

<script>
import TelemVarDefinitions from './data/TelemVarDefinitions';
import YattsAccordion from './components/YattsAccordion';
import Component1 from './components/Component1';
import Component2 from './components/Component2';
import { markRaw } from 'vue';

export default {
  name: 'App',
  data() {
    return {
      TelemVarDefinitions: TelemVarDefinitions,
      AccordionElements: [
        {
          type: markRaw(Component2),
          props: {
            uniqAttr2: 'world'
          }
        },
        {
          type: markRaw(YattsAccordion),
          props: {
            elements: [
              {
                type: markRaw(Component1),
                props: {
                  uniqAttr1: 'nested 1 (accordion of accordions)'
                }
              },
              {
                type: markRaw(Component2),
                props: {
                  uniqAttr2: 'nested 2 (accordion of accordions)'
                }
              }
            ]
          }
        }
      ]
    }
  },
  components: {
    YattsAccordion,
    // used in dynamic manner as a component of YattsAccordion
    // eslint-disable-next-line vue/no-unused-components
    Component1,
    // used in dynamic manner as a component of YattsAccordion
    // eslint-disable-next-line vue/no-unused-components
    Component2
  }
}
</script>
