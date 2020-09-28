<template>
  <div class="row mb-2" v-for="category in state" v-bind:key="category.def.id">
    <div class="col card bg-1">
      <div class="row">
        <div class="col">
          Category: {{ category.def.name }}, desc: {{ category.def.description }}
        </div>
      </div>
      <div class="row">
        <div class="col">
          <div class="row" v-for="group in category.groups" v-bind:key="group.def.id">
            <div class="col card m-2 bg-2">
              <div class="row">
                <div class="col">
                  Group: {{ group.def.name }}
                </div>
              </div>
              <div class="row" v-for="variable in group.variables" v-bind:key="variable.def.id">
                <div class="col card m-2 bg-3">
                  Variable: {{ variable.def.id }}
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
export default {
  name: 'TelemVarCatalogue',
  props: ['state']
}
</script>

<style lang="scss" scoped>
  $base-color: #007bff;

  .bg-1 {
    background-color: lighten($base-color, 10%);
  }

  .bg-2 {
    background-color: lighten($base-color, 20%);
  }

  .bg-3 {
    background-color: lighten($base-color, 30%);
  }
</style>
