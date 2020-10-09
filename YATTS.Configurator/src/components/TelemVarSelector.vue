<template>
  <div class="row card p-2 bg-grey">
    <div class="col">
      <div class="row margin-between">
        <input v-model="query" type="text" id="search" class="card" placeholder="Search for a variable by its name">
      </div>
      <div class="row disable-selection card margin-between bg-3" v-for="category in filteredConfig" v-bind:key="category.value.def.id">
        <div class="col">
          <div class="row" v-on:click="$emit('toggle-show', category.value)">
            <div class="col">
              <div class="category">
                <div class="category-left">
                  <span class="category-arrow"><RotatingArrow v-bind:rotated="category.value.show" /></span>
                  <span class="category-name">{{ category.value.def.name }}</span>
                  <span class="category-desc">{{ category.value.def.desc }}</span>
                </div>
                <div class="category-right">
                  <span class="category-count">{{ getCategoryVariableCount(category) }}</span>
                </div>
              </div>
            </div>
          </div>
          <div class="row indent-groups" v-if="category.value.show">
            <div class="col">
              <div class="row" v-for="group in category.groups" v-bind:key="group.value.def.id">
                <div class="col">
                  <div class="row" v-on:click="$emit('toggle-show', group.value)">
                    <div class="col">
                      <div class="group">
                        <div class="group-left">
                          <span class="group-arrow"><RotatingArrow v-bind:rotated="group.value.show" /></span>
                          <span class="group-name">{{ group.value.def.name }}</span>
                        </div>
                        <div class="group-right">
                          <span class="group-count">{{ group.variables.length }}</span>
                        </div>
                      </div>
                    </div>
                  </div>
                  <div class="row indent-variables" v-if="group.value.show">
                    <div class="col">
                      <div class="row" v-for="variable in group.variables" v-bind:key="variable.value.def.id">
                        <div class="col">
                          <div class="variable">
                            <div class="variable-left">
                              <input class="variable-selected" type="checkbox" v-model="variable.value.selected">
                              <span class="variable-name">{{ variable.value.def.name }}</span>
                            </div>
                            <div class="variable-right">
                              <span class="variable-id">{{ variable.value.def.id }}</span>
                            </div>
                          </div>
                        </div>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { ref, computed } from 'vue';

import RotatingArrow from '@/components/RotatingArrow';

import { FilterConfigByVariableName } from '@/utils/TelemVarConfigUtil';

export default {
  name: 'TelemVarSelector',
  props: ['config'],
  emits: ['toggle-show'],
  components: {
    RotatingArrow
  },
  setup(props) {
    const query = ref('');

    const sanitizedQuery = computed(() =>
      query.value.trim().toLowerCase()
    );

    const filteredConfig = computed(() =>
      sanitizedQuery.value !== '' ? FilterConfigByVariableName(props.config, sanitizedQuery.value) : props.config
    );

    const getCategoryVariableCount = category => category.groups.reduce((acc, group) => acc + group.variables.length, 0);

    const mounted = () => {
      console.log('siemka');
    }

    return { query, sanitizedQuery, filteredConfig, getCategoryVariableCount, mounted };
  }
}
</script>

<style lang="scss" scoped>
  .category {
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  .group {
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  .group-arrow {
    margin-right: .5rem;
  }

  .variable {
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  .variable-id {
    font-family: 'Courier New', Courier, monospace;
  }

  .variable-selected {
    margin-right: .5rem;
  }

  .search-card {
    display: flex;
    flex-direction: row;
  }

  #search {
    font-size: x-large;
    width: 100%;
    line-height: 1.5;
    padding-left: .5rem;
  }

  .indent-groups {
    padding-left: 1.3rem;
  }

  .indent-variables {
    padding-left: 1rem;
  }

  .disable-selection {
    user-select: none;
  }

  .category-arrow {
    font-size: medium;
    margin-right: .5rem;
    vertical-align: 12%;
  }

  .category-name {
    font-size: x-large;
    margin-right: .5rem;
  }

  .category-desc {
    font-size: medium;
  }
</style>
