import { markRaw } from 'vue';

export const GenerateInitialState = telemVarDefinitions => 
  telemVarDefinitions.map(category => ({
    def: markRaw(category),
    show: false,
    groups: category.groups.map(group => ({
      def: markRaw(group),
      show: false,
      variables: group.variables.map(variable => ({
        def: markRaw(variable),
        selected: false,
        maxCount: null
      }))
    }))
  }));

export const GenerateStateFromConfig = function(telemVarDefinitions, config) {
  return [];
}
