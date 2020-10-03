import { reactive, markRaw } from 'vue';

export const GetInitialConfig = telemVarDefinitions => 
  reactive(telemVarDefinitions.map(category => ({
    value: {
      def: markRaw(category.def),
      show: false
    },
    groups: category.groups.map(group => ({
      value: {
        def: markRaw(group.def),
        show: false
      },
      variables: group.variables.map(variable => ({
        value: {
          def: markRaw(variable),
          selected: false,
          maxCount: null
        }
      }))
    }))
  })));

export const FilterConfigByVariableName = (config, query) => 
  reactive(config.map(origCategory => ({
    value: origCategory.value,
    groups: origCategory.groups.map(origGroup => ({
      value: origGroup.value,
      variables: origGroup.variables.filter(variable => variable.value.def.id.toLowerCase().includes(query))
    })).filter(filteredGroup => filteredGroup.variables.length > 0)
  })).filter(filteredCategory => filteredCategory.groups.length > 0));

export const GenerateConfigFromConfigFile = (telemVarDefinitions, configFile) => { };
