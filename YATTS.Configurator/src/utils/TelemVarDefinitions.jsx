export default [
  {
    def: {
      id: 'channel',
      name: 'Channel variables',
      description: 'streamed continuously'
    },
    groups: [
      {
        def: {
          id: 'common',
          name: 'Common'
        },
        variables: [
          {
            id: 'local.scale',
            type: 5,
            desc: () => <>
              <p>This is a paragraph.</p>
              <p>This is an another paragraph.</p>
            </>
          },
          {
            id: 'game.time',
            type: 3,
            desc: () => <>
              this is just a text literal
            </>
          }
        ]
      },
      {
        def: {
          id: 'truck',
          name: 'Truck'
        },
        variables: [
          {
            id: 'speed',
            type: 5
          },
          {
            id: 'blinker.left',
            type: 1
          }
        ]
      },
      {
        def: {
          id: 'trailer',
          name: 'Trailer'
        },
        variables: [
          {
            id: 'connected',
            type: 1
          },
          {
            id: 'wheel.speed',
            type: 5
          }
        ]
      }
    ]
  },
  {
    def: {
      id: 'config',
      name: 'Config variables',
      description: 'sent only, when a group\'s variable changes'
    },
    groups: [
      {
        def: {
          id: 'truck',
          name: 'Truck'
        },
        variables: [
          {
            id: 'id',
            type: 15
          },
          {
            id: 'registration.plate',
            type: 15
          }
        ]
      }
    ]
  },
  {
    def: {
      id: 'event',
      name: 'Event variables',
      description: 'sent only, when an event occurs'

    },
    groups: []
  }
]
